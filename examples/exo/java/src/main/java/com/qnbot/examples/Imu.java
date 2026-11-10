package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.exo.ExoDevice;
import com.qnbot.sdk.exo.ExoImuState;

/**
 * Prints supported Exo IMU samples.
 *
 * <p>Supply an Android {@link Context}, an authorized {@link SerialTransport} and the device port;
 * the device must report IMU capability.
 */
public final class Imu {
  private Imu() {}

  public static void run(Context context, SerialTransport transport, String port) {
    SdkConfig config = SdkConfig.builder()
                           .device(ExoConfig.builder()
                                       .connection(SerialConnection.port(port))
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport); Exo exo = sdk.exo()) {
      ExoDevice device = exo.device();
      if (!device.getDeviceInfo().capabilities.imus) {
        System.out.println("Device has no IMU capability");
        return;
      }
      // The payload carries a torso IMU and an optional extra IMU; print whichever is present.
      device.telemetry().subscribe(sample -> {
        for (ExoImuState imu :
            new ExoImuState[] {sample.value.payload.torsoImu, sample.value.payload.extraImu}) {
          if (imu != null)
            System.out.println("IMU sequence=" + sample.sequence + " acceleration="
                + imu.accelerationMps2 + " angularVelocity=" + imu.angularVelocityRadS
                + " quaternion=" + imu.orientationXyzw);
        }
      });
      exo.start();
      exo.runForever();
    }
  }
}
