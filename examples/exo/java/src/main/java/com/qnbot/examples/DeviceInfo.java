package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.ExoSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.exo.ExoDeviceInfo;

/** DeviceInfo: the SDK workflow, invoked by an existing Android consumer. */
public final class DeviceInfo {
  private DeviceInfo() {}

  public static void run(Context context, SerialTransport transport, String port) throws Exception {
    SdkConfig config =
        SdkConfig.builder()
            .device(ExoConfig.builder().connection(SerialConnection.port(port)).build())
            .build();
    try (ExoSdk sdk = new ExoSdk(config, context, transport); Exo exo = sdk.exo()) {
      ExoDeviceInfo info = exo.device().getDeviceInfo();
      System.out.println("product=" + info.product + " protocol=" + info.protocolVersion.major + "."
          + info.protocolVersion.minor + " serial=" + info.serialNumber
          + " hardware=" + info.hardwareName);
      System.out.println("firmware=" + info.firmware.platform + "." + info.firmware.revision + "."
          + info.firmware.feature + "." + info.firmware.build);
      System.out.println("encoders=" + info.components.encoders.available + "/"
          + info.components.encoders.totalSlots + " handsets=" + info.components.handsets.available
          + "/" + info.components.handsets.totalSlots + " imus=" + info.components.imus.available
          + "/" + info.components.imus.totalSlots);
      System.out.println("capabilities telemetry=" + info.capabilities.telemetry
          + " encoders=" + info.capabilities.encoders + " handsets=" + info.capabilities.handsets
          + " imus=" + info.capabilities.imus + " haptics=" + info.capabilities.haptics
          + " wireless=" + info.capabilities.wireless);
    }
  }
}
