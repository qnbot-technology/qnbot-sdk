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
import com.qnbot.sdk.exo.ExoDeviceInfo;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;

/** DiscoverExoGlove: the SDK workflow, invoked by an existing Android consumer. */
public final class DiscoverExoGlove {
  private DiscoverExoGlove() {}

  public static void run(Context context, SerialTransport transport, String port) throws Exception {
    System.out.println("USB candidates=" + transport.inventory());
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
      System.out.println("left source=" + glove.device(Side.LEFT).sourceId());
      System.out.println("right source=" + glove.device(Side.RIGHT).sourceId());
      ExoDeviceInfo info = exo.device().getDeviceInfo();
      System.out.println("Exo product=" + info.product + " serial=" + info.serialNumber
          + " hardware=" + info.hardwareName);
    }
  }
}
