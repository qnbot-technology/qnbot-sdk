package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.ExoSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.exo.ExoDevice;
import com.qnbot.sdk.exo.ExoDeviceInfo;

/** DiscoverExos: the SDK workflow, invoked by an existing Android consumer. */
public final class DiscoverExos {
  private DiscoverExos() {}

  public static void run(Context context, SerialTransport transport, String port) throws Exception {
    System.out.println("USB candidates=" + transport.inventory());
    SdkConfig config =
        SdkConfig.builder()
            .device(ExoConfig.builder().connection(SerialConnection.port(port)).build())
            .build();
    try (ExoSdk sdk = new ExoSdk(config, context, transport); Exo exo = sdk.exo()) {
      ExoDevice device = exo.device();
      ExoDeviceInfo info = device.getDeviceInfo();
      System.out.println("selected port=" + port + " source=" + device.sourceId()
          + " product=" + info.product + " serial=" + info.serialNumber);
    }
  }
}
