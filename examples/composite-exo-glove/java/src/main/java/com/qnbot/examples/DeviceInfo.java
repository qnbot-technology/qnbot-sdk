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
import com.qnbot.sdk.exo.ExoDevice;
import com.qnbot.sdk.exo.ExoDeviceInfo;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;

/** DeviceInfo: the SDK workflow, invoked by an existing Android consumer. */
public final class DeviceInfo {
  private DeviceInfo() {}

  public static void run(Context context, SerialTransport transport, String port) throws Exception {
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
