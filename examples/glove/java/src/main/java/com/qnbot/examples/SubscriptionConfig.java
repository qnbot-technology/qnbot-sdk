package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.channel.ReadChannel;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GlovePose;
import java.util.Locale;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Counts pose deliveries under a configured subscription policy.
 *
 * <p>Pass the host {@link Context}, an authorized transport, the glove port and side, the positive
 * run time, and a latest/FIFO subscription configuration. The SDK background thread delivers
 * poses; the summary compares received samples with observed delivery rate.
 */
public final class SubscriptionConfig {
  private SubscriptionConfig() {}

  public static void run(Context context, SerialTransport transport, String port, Side side,
      double seconds, com.qnbot.sdk.channel.SubscriptionConfig subscription)
      throws InterruptedException {
    if (!Double.isFinite(seconds) || seconds <= 0)
      throw new IllegalArgumentException("seconds must be positive and finite");
    SdkConfig config = SdkConfig.builder()
        .device(GloveConfig.builder()
                    .name("primary")
                    .side(side)
                    .connection(SerialConnection.port(port))
                    .build())
        .build();
    AtomicInteger count = new AtomicInteger();
    AtomicLong firstAt = new AtomicLong();
    AtomicLong lastAt = new AtomicLong();
    AtomicLong lastSequence = new AtomicLong();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      ReadChannel<GlovePose> pose = glove.device().pose();
      pose.withConfig(subscription).subscribe(sample -> {
        long now = System.nanoTime();
        firstAt.compareAndSet(0, now);
        lastAt.set(now);
        count.incrementAndGet();
        lastSequence.set(sample.sequence);
      });
      glove.start();
      glove.runBackground();
      try {
        Thread.sleep((long) (seconds * 1_000));
      } finally {
        glove.requestStop();
        glove.join();
      }
    }
    double observedHz = count.get() > 1
        ? (count.get() - 1) / ((lastAt.get() - firstAt.get()) / 1_000_000_000.0)
        : 0.0;
    System.out.printf(Locale.ROOT, "samples=%d observed_hz=%.2f last_sequence=%d%n", count.get(),
        observedHz, lastSequence.get());
    System.out.println("subscription complete");
  }
}
