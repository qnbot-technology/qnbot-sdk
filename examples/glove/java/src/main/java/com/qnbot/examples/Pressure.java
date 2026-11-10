package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;

/**
 * Reads raw pressure readings from a pressure-capable glove.
 *
 * <p>Pass the host {@link Context}, an authorized {@link SerialTransport} and the glove's serial
 * port. Run on a worker thread; the pressure subscription prints one sample per second.
 */
public final class Pressure {
  private Pressure() {}

  public static void run(Context context, SerialTransport transport, String port) {
    SdkConfig config = SdkConfig.builder()
                           .device(GloveConfig.builder()
                                       .connection(SerialConnection.port(port))
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      device.pressure()
          .withConfig(com.qnbot.sdk.channel.SubscriptionConfig.latest(1.0))
          .subscribe(sample -> System.out.println("pressure sequence=" + sample.sequence
              + " points=" + sample.value.readings.size() + " first_raw="
              + sample.value.readings.get(0)));
      glove.start();
      glove.runForever();
    }
  }
}
