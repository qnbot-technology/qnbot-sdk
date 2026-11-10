package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.CompositeExoGloveConfig;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.exo.ExoDevice;
import com.qnbot.sdk.exo.ExoImuState;
import com.qnbot.sdk.glove.GloveConfig;

/**
 * Reads the Exo IMUs reported by a composite Exo+Glove device. The shared link does not expose raw
 * glove IMU, so this example checks the Exo capability and subscribes to Exo telemetry only.
 */
public final class Imu {
  private Imu() {}

  public static void run(Context context, SerialTransport transport, String port) {
    SdkConfig config = SdkConfig.builder()
                           .device(CompositeExoGloveConfig.builder()
                                       .connection(SerialConnection.port(port))
                                       .devices(GloveConfig.builder().side(Side.LEFT).build(),
                                           GloveConfig.builder().side(Side.RIGHT).build(),
                                           ExoConfig.builder().build())
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport)) {
      Exo exo = sdk.exo();
      ExoDevice exoDevice = exo.device();
      if (!exoDevice.getDeviceInfo().capabilities.imus) {
        System.out.println("Exo has no IMU capability; composite Glove raw IMU is unavailable");
        return;
      }
      exoDevice.telemetry().subscribe(sample -> {
        for (ExoImuState imu :
            new ExoImuState[] {sample.value.payload.torsoImu, sample.value.payload.extraImu}) {
          if (imu != null)
            System.out.println("IMU sequence=" + sample.sequence + " acceleration="
                + imu.accelerationMps2 + " angularVelocity=" + imu.angularVelocityRadS
                + " quaternion=" + imu.orientationXyzw);
        }
      });
      sdk.start();
      sdk.runForever();
    }
  }
}
