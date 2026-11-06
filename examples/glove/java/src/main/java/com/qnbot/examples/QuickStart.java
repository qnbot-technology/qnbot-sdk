package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.SleepResult;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.GloveFinger;
import java.util.function.BooleanSupplier;

/**
 * Streams the only discovered glove's pose until the caller stops.
 *
 * <p>Pass the host {@link Context}, an authorized {@link SerialTransport}, {@code null} for auto
 * discovery or a serial port, the physical {@link Side}, and a non-blocking stop signal. Run on a
 * worker thread; the pose subscription prints each sequence.
 */
public final class QuickStart {
  private QuickStart() {}

  public static void run(Context context, SerialTransport transport, String port, Side side,
      BooleanSupplier cancelled) throws InterruptedException {
    if (cancelled.getAsBoolean())
      return;
    SerialConnection connection =
        port == null ? SerialConnection.autoDiscover() : SerialConnection.port(port);
    SdkConfig config = SdkConfig.builder()
        .device(GloveConfig.builder().side(side).connection(connection).build())
        .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      device.pose().subscribe(sample -> System.out.println("pose sequence=" + sample.sequence
              + " thumb=" + sample.value.payload.fingertipLocal.get(GloveFinger.THUMB).position
              + " index=" + sample.value.payload.fingertipLocal.get(GloveFinger.INDEX).position));
      glove.start();
      while (!cancelled.getAsBoolean()) {
        var update = glove.update();
        if (!update.hasNextTask) {
          Thread.sleep(20);
        } else if (update.sleep() == SleepResult.STOPPED) {
          break;
        }
      }
    }
  }
}
