package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.GloveSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.CompositeExoGloveConfig;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.exo.ExoDevice;
import com.qnbot.sdk.exo.ExoImuState;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;

/** Imu: the SDK workflow, invoked by an existing Android consumer. */
public final class Imu {
  private Imu() {}

  public static void run(Context context, SerialTransport transport, String port) throws Exception {
    SdkConfig config = SdkConfig.builder()
                           .device(CompositeExoGloveConfig.builder()
                                       .connection(SerialConnection.port(port))
                                       .devices(GloveConfig.builder().side(Side.LEFT).build(),
                                           GloveConfig.builder().side(Side.RIGHT).build(),
                                           ExoConfig.builder().build())
                                       .build())
                           .build();
    try (GloveSdk sdk = new GloveSdk(config, context, transport)) {
      Glove glove = sdk.glove();
      Exo exo = new Exo(sdk);
      GloveDevice left = glove.device(Side.LEFT);
      GloveDevice right = glove.device(Side.RIGHT);
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
