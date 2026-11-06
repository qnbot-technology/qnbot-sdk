package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.channel.ReadChannel;
import com.qnbot.sdk.config.AlgorithmsConfig;
import com.qnbot.sdk.config.DeviceSelector;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.config.TargetAlgorithm;
import com.qnbot.sdk.config.TargetConfig;
import com.qnbot.sdk.glove.ExternalConnection;
import com.qnbot.sdk.glove.ExternalGloveFrame;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.GloveNodePose;
import com.qnbot.sdk.glove.calibration.CalibrationJobState;
import com.qnbot.sdk.glove.calibration.CalibrationProgress;
import com.qnbot.sdk.glove.capture.CaptureConfig;
import com.qnbot.sdk.glove.capture.CaptureControl;
import com.qnbot.sdk.glove.capture.CaptureInteractionMode;
import com.qnbot.sdk.glove.capture.CaptureProgress;
import com.qnbot.sdk.glove.capture.CaptureSessionState;
import com.qnbot.sdk.glove.capture.CaptureStageState;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.function.BooleanSupplier;
import java.util.function.Predicate;
import java.util.function.Supplier;

/**
 * Pushes application-owned glove frames while the SDK runs in the background, then prints the
 * first retargeting output.
 *
 * <p>Supply the Android Context, transport, side, package ID, Capture interaction and confirm
 * callback. Pass null frames for the built-in synthetic input; the loop ends after 60 seconds,
 * on cancellation, or when the frame supplier returns null.
 */
public final class ExternalInputRunBackground {
  private ExternalInputRunBackground() {}

  /**
   * confirm must return promptly: true confirms the displayed action, false leaves it pending.
   * cancelled stops the frame loop; both callbacks run on the caller thread.
   */
  public static void run(Context context, SerialTransport transport, Side side, String packageId,
      CaptureConfig captureConfig, Supplier<ExternalGloveFrame> frames, Predicate<String> confirm,
      BooleanSupplier cancelled) throws Exception {
    SdkConfig config = SdkConfig.builder()
                           .device(GloveConfig.builder()
                                       .name("primary")
                                       .side(side)
                                       .connection(ExternalConnection.INSTANCE)
                                       .build())
                           .algorithms(new AlgorithmsConfig(null, captureConfig))
                           .target(TargetConfig.builder()
                                       .name("openxr_hand")
                                       .side(side)
                                       .source(new DeviceSelector("glove", "primary", side))
                                       .algorithm(new TargetAlgorithm(packageId))
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      var output = device.output("openxr_hand");
      ReadChannel<CaptureProgress> captureProgress = device.captureProgress();
      ReadChannel<CalibrationProgress> calibrationProgress =
          device.calibrationProgress("openxr_hand");
      CaptureControl captureControl = device.captureControl();
      Set<String> handledRequestIds = new HashSet<>();
      glove.start();
      glove.runBackground();
      try {
        long deadline = System.nanoTime() + 60_000_000_000L;
        int step = 1;
        while (!cancelled.getAsBoolean() && System.nanoTime() < deadline
            && output.latest().isEmpty()) {
          failIfPreparationFailed(captureProgress, calibrationProgress);
          ExternalGloveFrame input = frames == null ? frame(step++) : frames.get();
          if (input == null) {
            break;
          }
          device.pushFrame(input);
          if (captureConfig.interaction == CaptureInteractionMode.EXTERNAL) {
            handleCapturePrompt(captureProgress, captureControl, handledRequestIds, confirm,
                device.name());
          }
          Thread.sleep(10);
        }
        if (cancelled.getAsBoolean()) {
          return;
        }
        var sample = output.latest().orElseThrow(
            ()
                -> new IllegalStateException("External input did not publish retargeting output "
                    + "before input ended or timed out"));
        System.out.println("latest retargeting output sequence=" + sample.sequence);
      } finally {
        glove.requestStop();
        glove.join();
      }
    }
    System.out.println("external background input complete");
  }

  private static void failIfPreparationFailed(ReadChannel<CaptureProgress> captureProgress,
      ReadChannel<CalibrationProgress> calibrationProgress) {
    captureProgress.latest().ifPresent(sample -> {
      var progress = sample.value;
      if (progress.sessionState == CaptureSessionState.FAILED
          || progress.sessionState == CaptureSessionState.CANCELLED) {
        throw new IllegalStateException(
            progress.failure == null ? "Capture stopped" : progress.failure.message);
      }
    });
    calibrationProgress.latest().ifPresent(sample -> {
      if (sample.value.job.state == CalibrationJobState.FAILED) {
        throw new IllegalStateException(sample.value.job.failure == null
                ? "Calibration stopped"
                : sample.value.job.failure.message);
      }
    });
  }

  private static void handleCapturePrompt(ReadChannel<CaptureProgress> captureProgress,
      CaptureControl captureControl, Set<String> handledRequestIds, Predicate<String> confirm,
      String sourceName) {
    captureProgress.latest().ifPresent(sample -> {
      var stage = sample.value.stage;
      if (stage.state == CaptureStageState.AWAITING_CONFIRMATION && stage.requestId != null
          && !handledRequestIds.contains(stage.requestId)
          && confirm.test(sourceName + " " + stage.prompt)) {
        captureControl.confirm(stage.requestId);
        handledRequestIds.add(stage.requestId);
      }
    });
  }

  private static ExternalGloveFrame frame(int step) {
    double offset = step * 0.001;
    return new ExternalGloveFrame(node(0.01 + offset, 0.02, 0.03), node(0.02 + offset, 0.03, 0.04),
        node(0.03 + offset, 0.04, 0.05), node(0.04 + offset, 0.05, 0.06),
        node(0.05 + offset, 0.06, 0.07), step * 10.0);
  }

  private static GloveNodePose node(double x, double y, double z) {
    return GloveNodePose.of(List.of(x, y, z), List.of(0.0, 0.0, 0.0, 1.0));
  }
}
