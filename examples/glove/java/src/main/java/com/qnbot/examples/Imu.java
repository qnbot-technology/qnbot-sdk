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

/**
 * Streams raw glove IMU counts.
 *
 * <p>Pass the host {@link Context}, an authorized transport, the glove port and side. Wired gloves
 * and compatible wireless receivers publish here; a sample with {@code valid=false} still prints
 * its raw gyroscope and accelerometer counts.
 */
public final class Imu {
  private Imu() {}

  public static void run(Context context, SerialTransport transport, String port, Side side) {
    SdkConfig config = SdkConfig.builder()
        .device(GloveConfig.builder()
                    .name("primary")
                    .side(side)
                    .connection(SerialConnection.port(port))
                    .build())
        .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      device.imu().subscribe(sample
          -> System.out.println("IMU sequence=" + sample.sequence + " valid="
              + sample.value.payload.valid + " gyroscope_raw=" + sample.value.payload.gyroscopeRaw
              + " accelerometer_raw=" + sample.value.payload.accelerometerRaw));
      glove.start();
      glove.runForever();
    }
  }
}
