package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.GloveSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
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
import com.qnbot.sdk.glove.capture.CaptureConfig;
import com.qnbot.sdk.glove.capture.CaptureInteractionMode;
import com.qnbot.sdk.glove.capture.CaptureSessionState;
import com.qnbot.sdk.glove.capture.CaptureStageState;
import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.BooleanSupplier;
import java.util.function.Predicate;
import org.json.JSONArray;
import org.json.JSONObject;

/** ExternalInputRunBackground: the SDK workflow, invoked by an existing Android consumer. */
public final class ExternalInputRunBackground {
  private ExternalInputRunBackground() {}

  public static void run(Context context, SerialTransport transport, Side side, String packageId,
      String framePath, Predicate<String> confirm, BooleanSupplier cancelled) throws Exception {
    if (framePath == null || framePath.isEmpty())
      throw new IllegalArgumentException("Provide a customer-owned five-finger JSONL file");
    SdkConfig config =
        SdkConfig.builder()
            .device(GloveConfig.builder()
                        .name("primary")
                        .side(side)
                        .connection(ExternalConnection.INSTANCE)
                        .build())
            .algorithms(new AlgorithmsConfig(
                null, new CaptureConfig(CaptureInteractionMode.EXTERNAL, false, "default")))
            .target(TargetConfig.builder()
                        .name("openxr_hand")
                        .side(side)
                        .source(new DeviceSelector("glove", "primary", side))
                        .algorithm(new TargetAlgorithm(packageId))
                        .build())
            .build();
    try (GloveSdk sdk = new GloveSdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      AtomicInteger outputs = new AtomicInteger();
      device.output("openxr_hand").subscribe(sample -> {
        outputs.incrementAndGet();
        System.out.println("output sequence=" + sample.sequence + " joints=" + sample.value.joints);
      });
      Set<String> captureConfirmed = new HashSet<>();
      glove.start();
      glove.runBackground();
      try (BufferedReader frames = new BufferedReader(
               new InputStreamReader(new FileInputStream(framePath), StandardCharsets.UTF_8))) {
        double previousTimestamp = Double.NEGATIVE_INFINITY;
        double firstTimestamp = 0;
        long started = 0;
        int submitted = 0;
        String line;
        while (!cancelled.getAsBoolean() && (line = frames.readLine()) != null) {
          ExternalGloveFrame frame;
          try {
            JSONObject value = new JSONObject(line);
            double timestamp = value.getDouble("timestamp_ms");
            if (!Double.isFinite(timestamp) || timestamp <= previousTimestamp)
              throw new IllegalArgumentException("timestamp_ms 必须有限且严格递增");
            JSONObject fingers = value.getJSONObject("fingers");
            List<GloveNodePose> poses = new ArrayList<>();
            for (String finger : List.of("thumb", "index", "middle", "ring", "pinky")) {
              JSONObject pose = fingers.getJSONObject(finger);
              JSONArray position = pose.getJSONArray("position");
              JSONArray rotation = pose.getJSONArray("quaternion_xyzw");
              if (position.length() != 3 || rotation.length() != 4)
                throw new IllegalArgumentException("position 需要 3 项，quaternion_xyzw 需要 4 项");
              poses.add(GloveNodePose.of(
                  List.of(position.getDouble(0), position.getDouble(1), position.getDouble(2)),
                  List.of(rotation.getDouble(0), rotation.getDouble(1), rotation.getDouble(2),
                      rotation.getDouble(3))));
            }
            frame = new ExternalGloveFrame(
                poses.get(0), poses.get(1), poses.get(2), poses.get(3), poses.get(4), timestamp);
            previousTimestamp = timestamp;
          } catch (Exception error) {
            throw new IllegalArgumentException(
                "外部帧第 " + (submitted + 1) + " 行无效: " + error.getMessage(), error);
          }
          if (submitted == 0) {
            firstTimestamp = frame.timestampMs;
            started = System.nanoTime();
          }
          do {
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
            if ((System.nanoTime() - started) / 1_000_000.0 >= frame.timestampMs - firstTimestamp)
              break;
            Thread.sleep(10);
          } while (!cancelled.getAsBoolean());
          if (cancelled.getAsBoolean())
            break;
          device.pushFrame(frame);
          submitted++;
        }
        System.out.println("submitted frames=" + submitted);
        long deadline = System.nanoTime() + 2_000_000_000L;
        while (!cancelled.getAsBoolean() && System.nanoTime() < deadline) {
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
                throw new java.util.concurrent.CancellationException("Capture cancelled by caller");
              }
              device.captureControl().confirm(stage.requestId);
              captureConfirmed.add(stage.requestId);
            }
          });
          Thread.sleep(20);
        }
        if (!cancelled.getAsBoolean() && outputs.get() == 0)
          throw new IllegalStateException(
              "No target output after input; check algorithm and capture prompts");
      } finally {
        glove.requestStop();
        glove.join();
      }
    }
  }
}
