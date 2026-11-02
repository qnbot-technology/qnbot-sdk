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

/** Imu: the SDK workflow, invoked by an existing Android consumer. */
public final class Imu {
  private Imu() {}

  public static void run(Context context, SerialTransport transport, String port, Side side)
      throws Exception {
    SdkConfig config = SdkConfig.builder()
                           .device(GloveConfig.builder()
                                       .name("primary")
                                       .side(side)
                                       .connection(SerialConnection.port(port))
                                       .build())
                           .build();
    try (GloveSdk sdk = new GloveSdk(config, context, transport); Glove glove = sdk.glove()) {
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
