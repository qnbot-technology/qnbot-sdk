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
import com.qnbot.sdk.exo.ExoDeviceInfo;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;

/**
 * Discovers a composite Exo+Glove device and prints the two glove sources and the Exo identity.
 * Callers supply the authorized transport; pass a shared port, or null to discover the only
 * authorized one. The transport inventory is listed before the Sdk is created.
 */
public final class DiscoverExoGlove {
  private DiscoverExoGlove() {}

  public static void run(Context context, SerialTransport transport, String port) throws Exception {
    // Android needs the authorized endpoint list before the SDK takes ownership of the transport.
    System.out.println("USB candidates=" + transport.inventory());
    SerialConnection connection = port == null
        ? SerialConnection.autoDiscover()
        : SerialConnection.port(port);
    SdkConfig config = SdkConfig.builder()
        .device(CompositeExoGloveConfig.builder()
            .connection(connection)
            .devices(GloveConfig.builder().side(Side.LEFT).build(),
                GloveConfig.builder().side(Side.RIGHT).build(), ExoConfig.builder().build())
            .build())
        .build();
    try (Sdk sdk = new Sdk(config, context, transport)) {
      // One root Sdk exposes both member domains for the shared link.
      Glove glove = sdk.glove();
      Exo exo = sdk.exo();
      System.out.println("left source=" + glove.device(Side.LEFT).sourceId());
      System.out.println("right source=" + glove.device(Side.RIGHT).sourceId());
      ExoDeviceInfo info = exo.device().getDeviceInfo();
      System.out.println("Exo product=" + info.product + " serial=" + info.serialNumber
          + " hardware=" + info.hardwareName);
    }
  }
}
