package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.ExoSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.exo.ExoDevice;
import com.qnbot.sdk.exo.ExoHaptics;

/** Haptics: the SDK workflow, invoked by an existing Android consumer. */
public final class Haptics {
  private Haptics() {}

  public static void run(Context context, SerialTransport transport, String port) throws Exception {
    SdkConfig config =
        SdkConfig.builder()
            .device(ExoConfig.builder().connection(SerialConnection.port(port)).build())
            .build();
    try (ExoSdk sdk = new ExoSdk(config, context, transport); Exo exo = sdk.exo()) {
      ExoDevice device = exo.device();
      if (!device.getDeviceInfo().capabilities.haptics) {
        System.out.println("Device has no haptics capability");
        return;
      }
      exo.start();
      try {
        device.haptics().set(new ExoHaptics(60, 60));
        Thread.sleep(1000);
      } finally {
        device.haptics().clear();
      }
    }
  }
}
