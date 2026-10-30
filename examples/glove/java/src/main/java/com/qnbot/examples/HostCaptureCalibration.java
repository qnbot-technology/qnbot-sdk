package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.GloveSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.channel.ReadChannel;
import com.qnbot.sdk.channel.Sample;
import com.qnbot.sdk.config.DeviceSelector;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.config.TargetAlgorithm;
import com.qnbot.sdk.config.TargetConfig;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.HandJointCommand;
import com.qnbot.sdk.glove.calibration.CalibrationJobProgress;
import com.qnbot.sdk.glove.calibration.CalibrationJobState;
import com.qnbot.sdk.glove.calibration.CalibrationMode;
import com.qnbot.sdk.glove.calibration.CalibrationResult;
import com.qnbot.sdk.glove.calibration.CalibrationSession;
import com.qnbot.sdk.glove.calibration.CalibrationSessionSnapshot;
import com.qnbot.sdk.glove.calibration.CalibrationSessionState;
import com.qnbot.sdk.glove.capture.CaptureControlAction;
import com.qnbot.sdk.glove.capture.CaptureSession;
import com.qnbot.sdk.glove.capture.CaptureSessionSnapshot;
import com.qnbot.sdk.glove.capture.CaptureSessionState;
import com.qnbot.sdk.glove.capture.CaptureStageProgress;
import com.qnbot.sdk.glove.capture.CaptureStageState;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.function.BooleanSupplier;
import java.util.function.Predicate;

/** HostCaptureCalibration: the SDK workflow, invoked by an existing Android consumer. */
public final class HostCaptureCalibration {
  private HostCaptureCalibration() {}

  public static void run(Context context, SerialTransport transport, String leftPort,
      String rightPort, List<String> packageIds, String selectedPackageId, boolean force,
      Predicate<String> confirm, BooleanSupplier cancelled) throws Exception {
    String builtin = "qnbot_hand.dynamic_openxr_hand";
    List<String> packages =
        packageIds.isEmpty() ? List.of(builtin) : new ArrayList<>(new LinkedHashSet<>(packageIds));
    Map<String, String> targets = new LinkedHashMap<>();
    int ordinary = (int) packages.stream().filter(id -> !id.equals(builtin)).count();
    int index = 0;
    for (String id : packages) {
      targets.put(id,
          id.equals(builtin)  ? "skeleton"
              : ordinary == 1 ? "openxr_hand"
                              : "openxr_hand_" + (++index));
    }
    if (selectedPackageId == null && packages.size() == 1)
      selectedPackageId = packages.get(0);
    String selectedTarget = targets.get(selectedPackageId);
    if (selectedTarget == null)
      throw new IllegalArgumentException("Select one configured package: " + packages);
    SdkConfig.Builder config = SdkConfig.builder()
                                   .device(GloveConfig.builder()
                                               .name("left")
                                               .side(Side.LEFT)
                                               .connection(SerialConnection.port(leftPort))
                                               .build())
                                   .device(GloveConfig.builder()
                                               .name("right")
                                               .side(Side.RIGHT)
                                               .connection(SerialConnection.port(rightPort))
                                               .build());
    for (Map.Entry<String, String> target : targets.entrySet()) {
      if (target.getKey().equals(builtin))
        continue;
      for (Side side : Side.values()) {
        config.target(
            TargetConfig.builder()
                .name(target.getValue())
                .side(side)
                .source(new DeviceSelector("glove", side == Side.LEFT ? "left" : "right", side))
                .algorithm(new TargetAlgorithm(target.getKey()))
                .build());
      }
    }
    try (GloveSdk sdk = new GloveSdk(config.build(), context, transport);
         Glove glove = sdk.glove()) {
      GloveDevice left = glove.device("left");
      GloveDevice right = glove.device("right");
      if (packages.contains(builtin)) {
        left.skeleton();
        right.skeleton();
      }
      glove.start(CalibrationMode.MANUAL);
      left.start(CalibrationMode.MANUAL);
      right.start(CalibrationMode.MANUAL);
      for (GloveDevice device : List.of(left, right)) {
        if (cancelled.getAsBoolean())
          throw new java.util.concurrent.CancellationException("Capture cancelled by caller");
        System.out.println(
            device.name() + " readiness=" + device.captureStatus(packages, "default").state);
        try (CaptureSession capture = device.startCapture(packages, "default", force)) {
          try {
            long captureDeadline = System.nanoTime() + 300_000_000_000L;
            Set<String> confirmed = new HashSet<>();
            long nextProgressAt = 0;
            while (!cancelled.getAsBoolean()) {
              CaptureSessionSnapshot snapshot = capture.snapshot();
              if (snapshot.state == CaptureSessionState.COMPLETED) {
                capture.result().orElseThrow(
                    () -> new IllegalStateException("Completed capture has no result"));
                break;
              }
              if (snapshot.state == CaptureSessionState.FAILED
                  || snapshot.state == CaptureSessionState.CANCELLED)
                throw new IllegalStateException(
                    snapshot.failure == null ? "Capture stopped" : snapshot.failure.message);
              if (System.nanoTime() >= nextProgressAt) {
                System.out.println(device.name() + " capture=" + snapshot.state + " stage="
                    + snapshot.currentStage + " samples=" + snapshot.collectedSampleCount + "/"
                    + snapshot.requiredSampleCount);
                nextProgressAt = System.nanoTime() + 500_000_000L;
              }
              if (snapshot.requestId != null && !confirmed.contains(snapshot.requestId)
                  && snapshot.allowedOperations.contains(CaptureControlAction.CONFIRM)) {
                for (CaptureStageProgress stage : snapshot.stageProgress) {
                  if (stage.stageId.equals(snapshot.currentStage)
                      && stage.state == CaptureStageState.AWAITING_CONFIRMATION) {
                    if (!confirm.test(device.name() + " " + stage.prompt))
                      throw new java.util.concurrent.CancellationException(
                          "Capture cancelled by caller");
                    capture.confirm(snapshot.requestId);
                    confirmed.add(snapshot.requestId);
                  }
                }
              }
              if (System.nanoTime() >= captureDeadline)
                throw new IllegalStateException("Capture timed out");
              glove.update().sleep();
              Thread.sleep(10);
            }
            if (cancelled.getAsBoolean())
              throw new java.util.concurrent.CancellationException("Capture cancelled by caller");
            try (CalibrationSession calibration =
                     device.startCalibration(capture, List.of(selectedTarget))) {
              try {
                long calibrationDeadline = System.nanoTime() + 300_000_000_000L;
                String previous = "";
                while (!cancelled.getAsBoolean()) {
                  CalibrationSessionSnapshot snapshot = calibration.snapshot();
                  StringBuilder progress = new StringBuilder("calibration=").append(snapshot.state);
                  for (CalibrationJobProgress job : snapshot.jobs) {
                    progress.append(" target=")
                        .append(job.targetId)
                        .append(" state=")
                        .append(job.state);
                    if (job.state == CalibrationJobState.FAILED)
                      throw new IllegalStateException(
                          job.failure == null ? "Calibration failed" : job.failure.message);
                  }
                  if (!previous.equals(progress.toString())) {
                    previous = progress.toString();
                    System.out.println(previous);
                  }
                  if (snapshot.state == CalibrationSessionState.COMPLETED) {
                    for (CalibrationResult result : calibration.result().orElseThrow(
                             ()
                                 -> new IllegalStateException(
                                     "Completed calibration has no results"))) {
                      System.out.println("saved calibration source=" + result.sourceId
                          + " target=" + result.targetId);
                    }
                    break;
                  }
                  if (snapshot.state == CalibrationSessionState.FAILED
                      || snapshot.state == CalibrationSessionState.CANCELLED)
                    throw new IllegalStateException("Calibration stopped: " + snapshot.state);
                  if (System.nanoTime() >= calibrationDeadline)
                    throw new IllegalStateException("Calibration timed out");
                  glove.update().sleep();
                  Thread.sleep(10);
                }
                if (cancelled.getAsBoolean())
                  throw new java.util.concurrent.CancellationException(
                      "Calibration cancelled by caller");
              } finally {
                if (calibration.snapshot().state == CalibrationSessionState.RUNNING)
                  calibration.cancel();
              }
            }
          } finally {
            CaptureSessionState state = capture.snapshot().state;
            if (state != CaptureSessionState.COMPLETED && state != CaptureSessionState.FAILED
                && state != CaptureSessionState.CANCELLED)
              capture.cancel();
          }
        }
        ReadChannel<HandJointCommand> output = selectedTarget.equals("skeleton")
            ? device.skeleton().jointAngles()
            : device.output(selectedTarget);
        long previous = output.latest().map(sample -> sample.sequence).orElse(-1L);
        long deadline = System.nanoTime() + 30_000_000_000L;
        while (!cancelled.getAsBoolean()) {
          glove.update().sleep();
          Optional<Sample<HandJointCommand>> sample = output.latest();
          if (sample.isPresent() && sample.get().sequence != previous) {
            System.out.println(device.name() + " calibrated target=" + sample.get().value.target
                + " joints=" + sample.get().value.joints);
            break;
          }
          if (System.nanoTime() >= deadline)
            throw new IllegalStateException("No fresh calibrated output");
        }
      }
    }
  }
}
