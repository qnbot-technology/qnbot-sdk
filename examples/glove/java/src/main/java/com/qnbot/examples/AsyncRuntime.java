package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.GloveSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.channel.ReadChannel;
import com.qnbot.sdk.channel.Sample;
import com.qnbot.sdk.config.AlgorithmsConfig;
import com.qnbot.sdk.config.DeviceSelector;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.config.TargetAlgorithm;
import com.qnbot.sdk.config.TargetConfig;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.HandJointCommand;
import com.qnbot.sdk.glove.calibration.CalibrationJobState;
import com.qnbot.sdk.glove.capture.CaptureConfig;
import com.qnbot.sdk.glove.capture.CaptureInteractionMode;
import com.qnbot.sdk.glove.capture.CaptureSessionState;
import com.qnbot.sdk.glove.capture.CaptureStageState;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Flow;
import java.util.concurrent.atomic.AtomicReference;
import java.util.function.BooleanSupplier;
import java.util.function.Predicate;

/** AsyncRuntime: the SDK workflow, invoked by an existing Android consumer. */
public final class AsyncRuntime {
  private AsyncRuntime() {}

  public static void run(Context context, SerialTransport transport, String port, Side side,
      String packageId, int samples, Predicate<String> confirm, BooleanSupplier cancelled)
      throws Exception {
    if (samples <= 0)
      throw new IllegalArgumentException("samples must be positive");
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
            .build();
    try (GloveSdk sdk = new GloveSdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      ReadChannel<HandJointCommand> output = device.output("openxr_hand");
      Set<String> captureConfirmed = new HashSet<>();
      CompletableFuture<Void> runner = new CompletableFuture<>();
      glove.start();
      glove.runBackground();
      Thread observer = new Thread(() -> {
        try {
          glove.join();
          runner.complete(null);
        } catch (RuntimeException error) {
          runner.completeExceptionally(error);
        }
      }, "glove-background-join");
      observer.start();
      CompletableFuture<Sample<HandJointCommand>> next = output.next().toCompletableFuture();
      AtomicReference<Flow.Subscription> active = new AtomicReference<>();
      try {
        while (!next.isDone() && !runner.isDone() && !cancelled.getAsBoolean()) {
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
        if (runner.isCompletedExceptionally())
          runner.get();
        if (cancelled.getAsBoolean())
          return;
        if (!next.isDone())
          throw new IllegalStateException("Runner ended before next output");
        System.out.println(
            "next sequence=" + next.get().sequence + " joints=" + next.get().value.joints);
        CompletableFuture<Void> streamed = new CompletableFuture<>();
        output.publisher().subscribe(new Flow.Subscriber<Sample<HandJointCommand>>() {
          private Flow.Subscription subscription;
          private int received;
          @Override
          public void onSubscribe(Flow.Subscription subscription) {
            this.subscription = subscription;
            active.set(subscription);
            subscription.request(1);
          }
          @Override
          public void onNext(Sample<HandJointCommand> sample) {
            try {
              System.out.println(
                  "Flow sequence=" + sample.sequence + " joints=" + sample.value.joints);
              if (++received == samples)
                streamed.complete(null);
              else
                subscription.request(1);
            } catch (RuntimeException error) {
              streamed.completeExceptionally(error);
            }
          }
          @Override
          public void onError(Throwable error) {
            streamed.completeExceptionally(error);
          }
          @Override
          public void onComplete() {
            streamed.complete(null);
          }
        });
        while (!streamed.isDone() && !runner.isDone() && !cancelled.getAsBoolean()) {
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
        if (runner.isCompletedExceptionally())
          runner.get();
        if (cancelled.getAsBoolean())
          return;
        if (!streamed.isDone())
          throw new IllegalStateException("Runner ended before stream completion");
        streamed.get();
      } finally {
        next.cancel(true);
        Flow.Subscription subscription = active.get();
        if (subscription != null)
          subscription.cancel();
        if (!runner.isDone())
          glove.requestStop();
        observer.join();
        runner.get();
      }
    }
  }
}
