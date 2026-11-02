package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.GloveSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.GloveDeviceInfo;

/** DiscoverGloves: the SDK workflow, invoked by an existing Android consumer. */
public final class DiscoverGloves {
  private DiscoverGloves() {}

  public static void run(Context context, SerialTransport transport, String port, Side side)
      throws Exception {
    System.out.println("USB candidates=" + transport.inventory());
    SdkConfig config = SdkConfig.builder()
                           .device(GloveConfig.builder()
                                       .name("primary")
                                       .side(side)
                                       .connection(SerialConnection.port(port))
                                       .build())
                           .build();
    try (GloveSdk sdk = new GloveSdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      GloveDeviceInfo info = device.getDeviceInfo();
      System.out.println("selected port=" + port + " source=" + device.sourceId() + " serial="
          + info.canonicalSn + " hand=" + info.hand + " transport=" + info.transportType);
    }
  }
}
