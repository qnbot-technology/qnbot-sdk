package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.ExoSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.exo.ExoDevice;
import com.qnbot.sdk.exo.ExoImuState;

/** Imu: the SDK workflow, invoked by an existing Android consumer. */
public final class Imu {
  private Imu() {}

  public static void run(Context context, SerialTransport transport, String port) throws Exception {
    SdkConfig config =
        SdkConfig.builder()
            .device(ExoConfig.builder().connection(SerialConnection.port(port)).build())
            .build();
    try (ExoSdk sdk = new ExoSdk(config, context, transport); Exo exo = sdk.exo()) {
      ExoDevice device = exo.device();
      if (!device.getDeviceInfo().capabilities.imus) {
        System.out.println("Device has no IMU capability");
        return;
      }
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
