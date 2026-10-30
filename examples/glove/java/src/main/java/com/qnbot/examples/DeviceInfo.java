package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.GloveSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDeviceInfo;

/** DeviceInfo: the SDK workflow, invoked by an existing Android consumer. */
public final class DeviceInfo {
  private DeviceInfo() {}

  public static void run(Context context, SerialTransport transport, String port, Side side)
      throws Exception {
    SdkConfig config = SdkConfig.builder()
                           .device(GloveConfig.builder()
                                       .name("primary")
                                       .side(side)
                                       .connection(SerialConnection.port(port))
                                       .build())
                           .build();
    try (GloveSdk sdk = new GloveSdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDeviceInfo info = glove.device().getDeviceInfo();
      System.out.println("sn=" + info.canonicalSn + " model=" + info.model + " firmware="
          + info.firmwareVersion + " side=" + info.hand + " transport=" + info.transportType);
    }
  }
}
