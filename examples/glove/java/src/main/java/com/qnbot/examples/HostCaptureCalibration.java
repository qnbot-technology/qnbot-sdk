package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.SleepResult;
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
import java.util.concurrent.CancellationException;
import java.util.function.BooleanSupplier;
import java.util.function.Predicate;

/**
 * Captures both gloves, calibrates one selected target and reads fresh calibrated output.
 *
 * <p>Supply the Android Context, transport, left/right ports, package IDs, target package and
 * confirm callback. Empty package IDs use the built-in skeleton; the SDK closes on every exit.
 */
public final class HostCaptureCalibration {
  private HostCaptureCalibration() {}

  /**
   * confirm must return promptly: true confirms the displayed action, false leaves it pending.
   * cancelled stops the current capture or calibration; both callbacks run on the caller thread.
   */
  public static void run(Context context, SerialTransport transport, String leftPort,
      String rightPort, List<String> packageIds, String selectedPackageId, boolean force,
      Predicate<String> confirm, BooleanSupplier cancelled) throws InterruptedException {
    String builtin = "qnbot_hand.dynamic_openxr_hand";
    List<String> packages =
        packageIds.isEmpty() ? List.of(builtin) : new ArrayList<>(new LinkedHashSet<>(packageIds));
    Map<String, String> targets = new LinkedHashMap<>();
    int ordinary = (int) packages.stream().filter(id -> !id.equals(builtin)).count();
    int index = 0;
    for (String id : packages) {
      targets.put(id, id.equals(builtin) ? "skeleton"
          : ordinary == 1 ? "openxr_hand" : "openxr_hand_" + (++index));
    }
    if (selectedPackageId == null && packages.size() == 1) {
      selectedPackageId = packages.get(0);
    }
    String selectedTarget = targets.get(selectedPackageId);
    if (selectedTarget == null) {
      throw new IllegalArgumentException("Select one configured package: " + packages);
    }
    SdkConfig.Builder config =
        SdkConfig.builder()
            .device(GloveConfig.builder()
                        .name("left")
                        .side(Side.LEFT)
                        .connection(leftPort == null ? SerialConnection.autoDiscover()
                                                     : SerialConnection.port(leftPort))
                        .build())
            .device(GloveConfig.builder()
                        .name("right")
                        .side(Side.RIGHT)
                        .connection(rightPort == null ? SerialConnection.autoDiscover()
                                                      : SerialConnection.port(rightPort))
                        .build());
    for (Map.Entry<String, String> target : targets.entrySet()) {
      if (target.getKey().equals(builtin)) {
        continue;
      }
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
    try (Sdk sdk = new Sdk(config.build(), context, transport); Glove glove = sdk.glove()) {
      GloveDevice left = glove.device("left");
      GloveDevice right = glove.device("right");
      if (packages.contains(builtin)) {
        left.skeleton();
        right.skeleton();
      }
      glove.start();
      left.start(CalibrationMode.MANUAL);
      right.start(CalibrationMode.MANUAL);
      if (cancelled.getAsBoolean()) {
        throw new CancellationException("Capture cancelled by caller");
      }
      try (CaptureSession leftCapture =
               captureSide(left, glove, packages, force, confirm, cancelled);
           CaptureSession rightCapture =
               captureSide(right, glove, packages, force, confirm, cancelled)) {
        calibrate(left, glove, leftCapture, selectedTarget, cancelled);
        calibrate(right, glove, rightCapture, selectedTarget, cancelled);
      }
      readTarget(left, glove, selectedTarget, cancelled);
      readTarget(right, glove, selectedTarget, cancelled);
    }
  }

  private static CaptureSession captureSide(GloveDevice device, Glove glove,
      List<String> packages, boolean force, Predicate<String> confirm, BooleanSupplier cancelled)
      throws InterruptedException {
    System.out.println(
        device.name() + " readiness=" + device.captureStatus(packages, "default").state);
    CaptureSession capture = device.startCapture(packages, "default", force);
    boolean completed = false;
    try {
      long deadline = System.nanoTime() + 300_000_000_000L;
      Set<String> confirmed = new HashSet<>();
      long nextProgressAt = 0;
      while (!cancelled.getAsBoolean()) {
        CaptureSessionSnapshot snapshot = capture.snapshot();
        if (snapshot.state == CaptureSessionState.COMPLETED) {
          capture.result().orElseThrow(
              () -> new IllegalStateException("Completed capture has no result"));
          completed = true;
          return capture;
        }
        if (snapshot.state == CaptureSessionState.FAILED
            || snapshot.state == CaptureSessionState.CANCELLED) {
          throw new IllegalStateException(
              snapshot.failure == null ? "Capture stopped" : snapshot.failure.message);
        }
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
              if (confirm.test(device.name() + " " + stage.prompt)) {
                capture.confirm(snapshot.requestId);
                confirmed.add(snapshot.requestId);
              }
            }
          }
        }
        if (System.nanoTime() >= deadline) {
          throw new IllegalStateException("Capture timed out");
        }
        var update = glove.update();
        if (!update.hasNextTask) {
          Thread.sleep(10);
        } else if (update.sleep() == SleepResult.STOPPED) {
          break;
        }
      }
      if (cancelled.getAsBoolean()) {
        throw new CancellationException("Capture cancelled by caller");
      }
      throw new IllegalStateException("Capture stopped");
    } finally {
      if (!completed) {
        try {
          CaptureSessionState state = capture.snapshot().state;
          if (state != CaptureSessionState.COMPLETED && state != CaptureSessionState.FAILED
              && state != CaptureSessionState.CANCELLED) {
            capture.cancel();
          }
        } finally {
          capture.close();
        }
      }
    }
  }

  private static void calibrate(GloveDevice device, Glove glove, CaptureSession capture,
      String target, BooleanSupplier cancelled) throws InterruptedException {
    try (CalibrationSession calibration = device.startCalibration(capture, List.of(target))) {
      try {
        long deadline = System.nanoTime() + 300_000_000_000L;
        String previous = "";
        while (!cancelled.getAsBoolean()) {
          CalibrationSessionSnapshot snapshot = calibration.snapshot();
          StringBuilder progress = new StringBuilder("calibration=").append(snapshot.state);
          for (CalibrationJobProgress job : snapshot.jobs) {
            progress.append(" target=").append(job.targetId).append(" state=").append(job.state);
            if (job.state == CalibrationJobState.FAILED) {
              throw new IllegalStateException(
                  job.failure == null ? "Calibration failed" : job.failure.message);
            }
          }
          if (!previous.equals(progress.toString())) {
            previous = progress.toString();
            System.out.println(previous);
          }
          if (snapshot.state == CalibrationSessionState.COMPLETED) {
            for (CalibrationResult result : calibration.result().orElseThrow(
                     () -> new IllegalStateException("Completed calibration has no results"))) {
              System.out.println("saved calibration source=" + result.sourceId
                  + " target=" + result.targetId);
            }
            return;
          }
          if (snapshot.state == CalibrationSessionState.FAILED
              || snapshot.state == CalibrationSessionState.CANCELLED) {
            throw new IllegalStateException("Calibration stopped: " + snapshot.state);
          }
          if (System.nanoTime() >= deadline) {
            throw new IllegalStateException("Calibration timed out");
          }
          var update = glove.update();
          if (!update.hasNextTask) {
            Thread.sleep(10);
          } else if (update.sleep() == SleepResult.STOPPED) {
            break;
          }
        }
        if (cancelled.getAsBoolean()) {
          throw new CancellationException("Calibration cancelled by caller");
        }
        throw new IllegalStateException("Calibration stopped");
      } finally {
        if (calibration.snapshot().state == CalibrationSessionState.RUNNING) {
          calibration.cancel();
        }
      }
    }
  }

  private static void readTarget(GloveDevice device, Glove glove, String target,
      BooleanSupplier cancelled) throws InterruptedException {
    ReadChannel<HandJointCommand> output =
        target.equals("skeleton") ? device.skeleton().jointAngles() : device.output(target);
    long previous = output.latest().map(sample -> sample.sequence).orElse(-1L);
    long deadline = System.nanoTime() + 30_000_000_000L;
    while (!cancelled.getAsBoolean()) {
      var update = glove.update();
      if (!update.hasNextTask) {
        Thread.sleep(20);
      } else if (update.sleep() == SleepResult.STOPPED) {
        break;
      }
      Optional<Sample<HandJointCommand>> sample = output.latest();
      if (sample.isPresent() && sample.get().sequence != previous) {
        System.out.println(device.name() + " calibrated target=" + sample.get().value.target
            + " joints=" + sample.get().value.joints);
        return;
      }
      if (System.nanoTime() >= deadline) {
        throw new IllegalStateException("No fresh calibrated output");
      }
    }
  }
}
