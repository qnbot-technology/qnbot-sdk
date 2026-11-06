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
import com.qnbot.sdk.exo.ExoDevice;
import com.qnbot.sdk.exo.ExoDeviceInfo;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;

/**
 * Shows the members of one composite Exo+Glove device: the two glove source IDs plus the Exo
 * product, firmware, protocol and capability details. Callers supply an authorized transport and
 * the shared port; the single Sdk is closed after the read.
 */
public final class DeviceInfo {
  private DeviceInfo() {}

  public static void run(Context context, SerialTransport transport, String port) {
    SdkConfig config = SdkConfig.builder()
        .device(CompositeExoGloveConfig.builder()
            .connection(SerialConnection.port(port))
            .devices(GloveConfig.builder().side(Side.LEFT).build(),
                GloveConfig.builder().side(Side.RIGHT).build(), ExoConfig.builder().build())
            .build())
        .build();
    try (Sdk sdk = new Sdk(config, context, transport)) {
      // One root Sdk exposes both member domains for the shared link.
      Glove glove = sdk.glove();
      Exo exo = sdk.exo();
      GloveDevice left = glove.device(Side.LEFT);
      GloveDevice right = glove.device(Side.RIGHT);
      ExoDevice exoDevice = exo.device();
      System.out.println("left source=" + left.sourceId());
      System.out.println("right source=" + right.sourceId());
      ExoDeviceInfo info = exoDevice.getDeviceInfo();
      System.out.println("product=" + info.product + " serial=" + info.serialNumber + " firmware="
          + info.firmware.platform + "." + info.firmware.revision + "." + info.firmware.feature
          + "." + info.firmware.build + " protocol=" + info.protocolVersion.major + "."
          + info.protocolVersion.minor + " encoders=" + info.components.encoders.available + "/"
          + info.components.encoders.totalSlots + " handsets=" + info.capabilities.handsets
          + " imus=" + info.capabilities.imus + " haptics=" + info.capabilities.haptics
          + " wireless=" + info.capabilities.wireless);
    }
  }
}
