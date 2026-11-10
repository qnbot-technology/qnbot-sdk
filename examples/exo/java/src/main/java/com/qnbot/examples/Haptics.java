package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.exo.ExoDevice;
import com.qnbot.sdk.exo.ExoHaptics;

/**
 * Sets Exo haptics for a hold duration, then clears them before the SDK closes.
 *
 * <p>Supply an Android {@link Context}, an authorized {@link SerialTransport}, the device port and
 * the left/right strengths; the device must report haptics capability.
 */
public final class Haptics {
  private Haptics() {}

  public static void run(Context context, SerialTransport transport, String port, int left,
      int right, double holdSeconds) throws Exception {
    if (!Double.isFinite(holdSeconds) || holdSeconds < 0)
      throw new IllegalArgumentException("holdSeconds must be finite and nonnegative");
    SdkConfig config = SdkConfig.builder()
                           .device(ExoConfig.builder()
                                       .connection(SerialConnection.port(port))
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport); Exo exo = sdk.exo()) {
      ExoDevice device = exo.device();
      if (!device.getDeviceInfo().capabilities.haptics) {
        System.out.println("Device has no haptics capability");
        return;
      }
      exo.start();
      // Clear in a finally block so an interrupted hold still releases the motors.
      try {
        device.haptics().set(new ExoHaptics(left, right));
        Thread.sleep((long) (holdSeconds * 1000));
      } finally {
        device.haptics().clear();
      }
    }
  }
}
