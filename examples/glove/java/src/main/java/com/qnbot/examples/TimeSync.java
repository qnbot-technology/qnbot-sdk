package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import java.util.function.DoubleSupplier;

/**
 * Runs in the background while resynchronizing Glove time every second.
 *
 * <p>Pass the host {@link Context}, an authorized transport and an external Unix-millisecond time
 * source. The SDK auto-discovers the only glove; pose and status subscriptions show the
 * synchronized timestamps. Interrupt the worker to stop and join the runner.
 */
public final class TimeSync {
  private TimeSync() {}

  /** Resynchronizes from the host clock. */
  public static void run(Context context, SerialTransport transport) throws InterruptedException {
    run(context, transport, System::currentTimeMillis);
  }

  public static void run(Context context, SerialTransport transport,
      DoubleSupplier referenceTimeMillis) throws InterruptedException {
    SdkConfig config = SdkConfig.builder().device(GloveConfig.builder().build()).build();
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
        while (true) {
          Thread.sleep(1_000);
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
