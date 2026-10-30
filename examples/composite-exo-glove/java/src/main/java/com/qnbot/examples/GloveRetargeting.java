package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.GloveSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.AlgorithmsConfig;
import com.qnbot.sdk.config.CompositeExoGloveConfig;
import com.qnbot.sdk.config.DeviceSelector;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.config.TargetAlgorithm;
import com.qnbot.sdk.config.TargetConfig;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.calibration.CalibrationJobState;
import com.qnbot.sdk.glove.capture.CaptureConfig;
import com.qnbot.sdk.glove.capture.CaptureInteractionMode;
import com.qnbot.sdk.glove.capture.CaptureSessionState;
import com.qnbot.sdk.glove.capture.CaptureStageState;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.atomic.AtomicReference;
import java.util.function.Predicate;

/** GloveRetargeting: the SDK workflow, invoked by an existing Android consumer. */
public final class GloveRetargeting {
  private GloveRetargeting() {}

  public static void run(Context context, SerialTransport transport, String port, String packageId,
      Predicate<String> confirm) throws Exception {
    SdkConfig config =
        SdkConfig.builder()
            .device(
                CompositeExoGloveConfig.builder()
                    .connection(SerialConnection.port(port))
                    .devices(GloveConfig.builder().side(Side.LEFT).build(),
                        GloveConfig.builder().side(Side.RIGHT).build(), ExoConfig.builder().build())
                    .build())
            .algorithms(new AlgorithmsConfig(
                null, new CaptureConfig(CaptureInteractionMode.EXTERNAL, false, "default")))
            .target(TargetConfig.builder()
                        .name("left-hand")
                        .side(Side.LEFT)
                        .source(new DeviceSelector("glove", null, Side.LEFT))
                        .algorithm(new TargetAlgorithm(packageId))
                        .build())
            .target(TargetConfig.builder()
                        .name("right-hand")
                        .side(Side.RIGHT)
                        .source(new DeviceSelector("glove", null, Side.RIGHT))
                        .algorithm(new TargetAlgorithm(packageId))
                        .build())
            .build();
    try (GloveSdk sdk = new GloveSdk(config, context, transport)) {
      Glove glove = sdk.glove();
      Exo exo = new Exo(sdk);
      GloveDevice left = glove.device(Side.LEFT);
      GloveDevice right = glove.device(Side.RIGHT);
      left.output("left-hand")
          .subscribe(sample
              -> System.out.println("output sequence=" + sample.sequence
                  + " target=" + sample.value.target + " joints=" + sample.value.joints));
      right.output("right-hand")
          .subscribe(sample
              -> System.out.println("output sequence=" + sample.sequence
                  + " target=" + sample.value.target + " joints=" + sample.value.joints));
      Set<String> leftCaptureConfirmed = new HashSet<>();
      Set<String> rightCaptureConfirmed = new HashSet<>();
      sdk.start();
      AtomicReference<RuntimeException> captureFailure = new AtomicReference<>();
      Thread controls = new Thread(() -> {
        try {
          while (!Thread.currentThread().isInterrupted()) {
            left.calibrationProgress("left-hand").latest().ifPresent(sample -> {
              if (sample.value.job.state == CalibrationJobState.FAILED
                  || sample.value.job.state == CalibrationJobState.CANCELLED) {
                throw new IllegalStateException(sample.value.job.failure == null
                        ? "Calibration stopped"
                        : sample.value.job.failure.message);
              }
            });
            left.captureProgress().latest().ifPresent(sample -> {
              var progress = sample.value;
              if (progress.sessionState == CaptureSessionState.FAILED
                  || progress.sessionState == CaptureSessionState.CANCELLED) {
                throw new IllegalStateException(
                    progress.failure == null ? "Capture stopped" : progress.failure.message);
              }
              var stage = progress.stage;
              if (stage.state == CaptureStageState.AWAITING_CONFIRMATION && stage.requestId != null
                  && !leftCaptureConfirmed.contains(stage.requestId)) {
                if (!confirm.test(left.name() + " " + stage.prompt)) {
                  throw new java.util.concurrent.CancellationException(
                      "Capture cancelled by caller");
                }
                left.captureControl().confirm(stage.requestId);
                leftCaptureConfirmed.add(stage.requestId);
              }
            });
            right.calibrationProgress("right-hand").latest().ifPresent(sample -> {
              if (sample.value.job.state == CalibrationJobState.FAILED
                  || sample.value.job.state == CalibrationJobState.CANCELLED) {
                throw new IllegalStateException(sample.value.job.failure == null
                        ? "Calibration stopped"
                        : sample.value.job.failure.message);
              }
            });
            right.captureProgress().latest().ifPresent(sample -> {
              var progress = sample.value;
              if (progress.sessionState == CaptureSessionState.FAILED
                  || progress.sessionState == CaptureSessionState.CANCELLED) {
                throw new IllegalStateException(
                    progress.failure == null ? "Capture stopped" : progress.failure.message);
              }
              var stage = progress.stage;
              if (stage.state == CaptureStageState.AWAITING_CONFIRMATION && stage.requestId != null
                  && !rightCaptureConfirmed.contains(stage.requestId)) {
                if (!confirm.test(right.name() + " " + stage.prompt)) {
                  throw new java.util.concurrent.CancellationException(
                      "Capture cancelled by caller");
                }
                right.captureControl().confirm(stage.requestId);
                rightCaptureConfirmed.add(stage.requestId);
              }
            });
            Thread.sleep(20);
          }
        } catch (InterruptedException stopped) {
          Thread.currentThread().interrupt();
        } catch (RuntimeException error) {
          captureFailure.set(error);
          sdk.requestStop();
        }
      }, "capture-confirmation");
      controls.start();
      try {
        sdk.runForever();
      } finally {
        controls.interrupt();
        controls.join();
      }
      if (captureFailure.get() != null)
        throw captureFailure.get();
    }
  }
}
