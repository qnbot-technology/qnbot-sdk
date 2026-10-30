package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.GloveSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.channel.ReadChannel;
import com.qnbot.sdk.channel.Sample;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GlovePose;
import java.util.concurrent.CompletableFuture;

/** SubscriptionConfig: the SDK workflow, invoked by an existing Android consumer. */
public final class SubscriptionConfig {
  private SubscriptionConfig() {}

  public static void run(Context context, SerialTransport transport, String port, Side side,
      double seconds) throws Exception {
    if (!Double.isFinite(seconds) || seconds <= 0)
      throw new IllegalArgumentException("seconds must be positive and finite");
    SdkConfig config = SdkConfig.builder()
                           .device(GloveConfig.builder()
                                       .name("primary")
                                       .side(side)
                                       .connection(SerialConnection.port(port))
                                       .build())
                           .build();
    try (GloveSdk sdk = new GloveSdk(config, context, transport); Glove glove = sdk.glove()) {
      ReadChannel<GlovePose> pose = glove.device().pose();
      pose.withConfig(com.qnbot.sdk.channel.SubscriptionConfig.latest(5.0))
          .subscribe(sample -> System.out.println("Latest 5Hz sequence=" + sample.sequence));
      pose.withConfig(com.qnbot.sdk.channel.SubscriptionConfig.fifo(128, 10.0))
          .subscribe(sample -> System.out.println("FIFO128 10Hz sequence=" + sample.sequence));
      glove.start();
      glove.runBackground();
      CompletableFuture<Sample<GlovePose>> next = pose.next().toCompletableFuture();
      try {
        System.out.println("next sequence=" + next.get().sequence);
        long deadline = System.nanoTime() + (long) (seconds * 1_000_000_000L);
        while (System.nanoTime() < deadline) {
          pose.latest().ifPresent(
              sample -> System.out.println("latest sequence=" + sample.sequence));
          Thread.sleep(100);
        }
      } finally {
        next.cancel(true);
        glove.requestStop();
        glove.join();
      }
    }
  }
}
