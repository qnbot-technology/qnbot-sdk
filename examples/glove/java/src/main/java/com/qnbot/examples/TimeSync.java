package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.GloveSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import java.util.function.DoubleSupplier;

/** TimeSync: the SDK workflow, invoked by an existing Android consumer. */
public final class TimeSync {
  private TimeSync() {}

  public static void run(Context context, SerialTransport transport, String port, Side side,
      DoubleSupplier referenceTimeMillis, double seconds) throws Exception {
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
      GloveDevice device = glove.device();
      device.pose().subscribe(sample
          -> System.out.println("sequence=" + sample.sequence
              + " source_time_ms=" + sample.sourceTime + " observed_at_ms=" + sample.observedAt));
      double reference = referenceTimeMillis.getAsDouble();
      glove.syncTime(reference);
      System.out.println("reference_time_ms=" + reference);
      glove.start();
      glove.runBackground();
      try {
        long deadline = System.nanoTime() + (long) (seconds * 1_000_000_000L);
        while (System.nanoTime() < deadline) {
          Thread.sleep(1000);
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
