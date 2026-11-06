package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import java.util.function.DoubleSupplier;

/**
 * Runs in the background while resynchronizing Glove time every second.
 *
 * <p>Pass the host {@link Context}, an authorized transport, the glove port and side, an external
 * Unix-millisecond time source, and the positive run time. Pose and status subscriptions show the
 * synchronized timestamps; cleanup stops and joins the runner.
 */
public final class TimeSync {
  private TimeSync() {}

  public static void run(Context context, SerialTransport transport, String port, Side side,
      DoubleSupplier referenceTimeMillis, double seconds) throws InterruptedException {
    if (!Double.isFinite(seconds) || seconds <= 0)
      throw new IllegalArgumentException("seconds must be positive and finite");
    SdkConfig config = SdkConfig.builder()
        .device(GloveConfig.builder()
                    .name("primary")
                    .side(side)
                    .connection(port == null ? SerialConnection.autoDiscover()
                                             : SerialConnection.port(port))
                    .build())
        .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      double reference = referenceTimeMillis.getAsDouble();
      glove.syncTime(reference);
      System.out.println("reference_time_ms=" + reference);
      device.pose().subscribe(sample
          -> System.out.println("pose sequence=" + sample.sequence
              + " source_time_ms=" + sample.sourceTime + " observed_at_ms=" + sample.observedAt));
      device.status().subscribe(sample
          -> System.out.println("status source_time_ms=" + sample.sourceTime
              + " observed_at_ms=" + sample.observedAt + " connected=" + sample.value.connected));
      glove.start();
      glove.runBackground();
      try {
        long deadline = System.nanoTime() + (long) (seconds * 1_000_000_000L);
        while (System.nanoTime() < deadline) {
          long remainingMillis = (deadline - System.nanoTime()) / 1_000_000L;
          Thread.sleep(Math.max(1, Math.min(1_000, remainingMillis)));
          reference = referenceTimeMillis.getAsDouble();
          glove.syncTime(reference);
          System.out.println("reference_time_ms=" + reference);
        }
      } finally {
        glove.requestStop();
        glove.join();
      }
    }
  }
}
