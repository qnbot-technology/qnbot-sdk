package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.GloveSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.AlgorithmsConfig;
import com.qnbot.sdk.config.DebugConfig;
import com.qnbot.sdk.config.DebugDetail;
import com.qnbot.sdk.config.DebugModule;
import com.qnbot.sdk.config.DeviceSelector;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.config.TargetAlgorithm;
import com.qnbot.sdk.config.TargetConfig;
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

/** DebugTrace: the SDK workflow, invoked by an existing Android consumer. */
public final class DebugTrace {
  private DebugTrace() {}

  public static void run(Context context, SerialTransport transport, String port, Side side,
      String packageId, Predicate<String> confirm) throws Exception {
    SdkConfig config =
        SdkConfig.builder()
            .device(GloveConfig.builder()
                        .name("primary")
                        .side(side)
                        .connection(SerialConnection.port(port))
                        .build())
            .algorithms(new AlgorithmsConfig(
                null, new CaptureConfig(CaptureInteractionMode.EXTERNAL, false, "default")))
            .target(TargetConfig.builder()
                        .name("openxr_hand")
                        .side(side)
                        .source(new DeviceSelector("glove", "primary", side))
                        .algorithm(new TargetAlgorithm(packageId))
                        .build())
            .debug(DebugConfig.builder()
                       .modules(DebugModule.values())
                       .detail(DebugDetail.SUMMARY)
                       .sampleRate(10)
                       .build())
            .build();
    try (GloveSdk sdk = new GloveSdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      device.output("openxr_hand")
          .subscribe(sample
              -> System.out.println("output sequence=" + sample.sequence
                  + " target=" + sample.value.target + " joints=" + sample.value.joints));
      Set<String> captureConfirmed = new HashSet<>();
      glove.start();
      AtomicReference<RuntimeException> captureFailure = new AtomicReference<>();
      Thread controls = new Thread(() -> {
        try {
          while (!Thread.currentThread().isInterrupted()) {
            device.calibrationProgress("openxr_hand").latest().ifPresent(sample -> {
              if (sample.value.job.state == CalibrationJobState.FAILED
                  || sample.value.job.state == CalibrationJobState.CANCELLED) {
                throw new IllegalStateException(sample.value.job.failure == null
                        ? "Calibration stopped"
                        : sample.value.job.failure.message);
              }
            });
            device.captureProgress().latest().ifPresent(sample -> {
              var progress = sample.value;
              if (progress.sessionState == CaptureSessionState.FAILED
                  || progress.sessionState == CaptureSessionState.CANCELLED) {
                throw new IllegalStateException(
                    progress.failure == null ? "Capture stopped" : progress.failure.message);
              }
              var stage = progress.stage;
              if (stage.state == CaptureStageState.AWAITING_CONFIRMATION && stage.requestId != null
                  && !captureConfirmed.contains(stage.requestId)) {
                if (!confirm.test(device.name() + " " + stage.prompt)) {
                  throw new java.util.concurrent.CancellationException(
                      "Capture cancelled by caller");
                }
                device.captureControl().confirm(stage.requestId);
                captureConfirmed.add(stage.requestId);
              }
            });
            Thread.sleep(20);
          }
        } catch (InterruptedException stopped) {
          Thread.currentThread().interrupt();
        } catch (RuntimeException error) {
          captureFailure.set(error);
          glove.requestStop();
        }
      }, "capture-confirmation");
      controls.start();
      try {
        glove.runForever();
      } finally {
        controls.interrupt();
        controls.join();
      }
      if (captureFailure.get() != null)
        throw captureFailure.get();
    }
  }
}
