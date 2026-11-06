package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
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
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Flow;
import java.util.concurrent.atomic.AtomicReference;
import java.util.function.BooleanSupplier;

/**
 * Reads one retargeted sample with next(), then a bounded Flow stream.
 *
 * <p>Pass the host {@link Context}, an authorized transport, the glove port and side, the installed
 * algorithm package, a positive sample count, and a non-blocking stop signal. The SDK background
 * thread advances work; the caller waits for the asynchronous reads and runner termination, then
 * requests stop and joins during cleanup.
 */
public final class AsyncRuntime {
  private AsyncRuntime() {}

  public static void run(Context context, SerialTransport transport, String port, Side side,
      String packageId, int samples, BooleanSupplier cancelled) throws Exception {
    if (cancelled.getAsBoolean())
      return;
    if (samples <= 0)
      throw new IllegalArgumentException("samples must be positive");
    SdkConfig config = SdkConfig.builder()
        .device(GloveConfig.builder()
                    .name("primary")
                    .side(side)
                    .connection(SerialConnection.port(port))
                    .build())
        .target(TargetConfig.builder()
                    .name("openxr_hand")
                    .side(side)
                    .source(new DeviceSelector("glove", "primary", side))
                    .algorithm(new TargetAlgorithm(packageId))
                    .build())
        .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      ReadChannel<HandJointCommand> output = device.output("openxr_hand");
      CompletableFuture<Void> streamed = new CompletableFuture<>();
      AtomicReference<Flow.Subscription> active = new AtomicReference<>();
      var subscriber = new Flow.Subscriber<Sample<HandJointCommand>>() {
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
            System.out.println("stream retargeting sequence=" + sample.sequence
                + " target=" + sample.value.target + " joints=" + sample.value.joints);
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
      };
      glove.start();
      glove.runBackground();
      CompletableFuture<Void> runner = CompletableFuture.runAsync(glove::join);
      var next = output.next().toCompletableFuture();
      try {
        while (!next.isDone() && !runner.isDone() && !cancelled.getAsBoolean()) {
          Thread.sleep(20);
        }
        if (runner.isCompletedExceptionally())
          runner.get();
        if (cancelled.getAsBoolean())
          return;
        if (!next.isDone())
          throw new IllegalStateException("Background run ended before the first output sample");
        Sample<HandJointCommand> first = next.get();
        System.out.println("next retargeting sequence=" + first.sequence
            + " target=" + first.value.target + " joints=" + first.value.joints);

        output.publisher().subscribe(subscriber);
        while (!streamed.isDone() && !runner.isDone() && !cancelled.getAsBoolean()) {
          Thread.sleep(20);
        }
        if (runner.isCompletedExceptionally())
          runner.get();
        if (cancelled.getAsBoolean())
          return;
        if (!streamed.isDone())
          throw new IllegalStateException(
              "Background run ended before output consumption finished");
        streamed.get();
      } finally {
        next.cancel(false);
        Flow.Subscription subscription = active.get();
        if (subscription != null)
          subscription.cancel();
        // Stop only while the background join is still running, then observe its result.
        if (!runner.isDone())
          glove.requestStop();
        runner.join();
      }
    }
  }
}
