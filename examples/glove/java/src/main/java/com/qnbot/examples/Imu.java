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

/** Streams unit-typed glove IMU samples. */
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
      device.imu().subscribe(sample -> {
        for (com.qnbot.sdk.glove.GloveImuSample imu : sample.value.payload.imus) {
          System.out.println("IMU id=" + imu.id + " location=" + imu.location
              + " valid=" + imu.valid + " fresh=" + imu.fresh);
          if (imu.values instanceof com.qnbot.sdk.glove.GloveImuValues.RawCounts) {
            com.qnbot.sdk.glove.GloveImuValues.RawCounts raw =
                (com.qnbot.sdk.glove.GloveImuValues.RawCounts) imu.values;
            System.out.println(
                "raw_counts gyro=" + raw.gyroscopeRaw + " accel=" + raw.accelerometerRaw);
          } else {
            com.qnbot.sdk.glove.GloveImuValues.Physical physical =
                (com.qnbot.sdk.glove.GloveImuValues.Physical) imu.values;
            System.out.println("gyro_mdps=" + physical.gyroMdps + " accel_mg=" + physical.accelMg);
          }
        }
      });
      glove.start();
      glove.runForever();
    }
  }
}
