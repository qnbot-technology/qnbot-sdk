package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.SleepResult;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.exo.ExoDevice;
import com.qnbot.sdk.exo.ExoImuState;
import java.util.function.BooleanSupplier;

/**
 * Prints supported Exo IMU samples while updating on the caller's worker.
 *
 * <p>Supply an Android {@link Context}, an authorized {@link SerialTransport} and the device
 * port; the device must report IMU capability. Return true from {@code cancelled} to stop.
 */
public final class Imu {
  private Imu() {}

  public static void run(Context context, SerialTransport transport, String port,
      BooleanSupplier cancelled) throws InterruptedException {
    if (cancelled.getAsBoolean())
      return;
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
      // Advance SDK work on this thread; the caller's stop signal ends the loop.
      while (!cancelled.getAsBoolean()) {
        var update = exo.update();
        if (!update.hasNextTask) {
          Thread.sleep(20);
        } else if (update.sleep() == SleepResult.STOPPED) {
          break;
        }
      }
    }
  }
}
