package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.GloveDeviceInfo;
import java.util.Locale;

/**
 * Reads one glove's identity, firmware and transport information before starting the runtime.
 *
 * <p>Supply the Android Context, an authorized transport and optional port/side; null selects
 * automatic discovery or leaves the side unconstrained. The SDK closes when the read completes.
 */
public final class DeviceInfo {
  private DeviceInfo() {}

  public static void run(Context context, SerialTransport transport, String port, Side side)
      throws Exception {
    SdkConfig config = SdkConfig.builder()
                           .device(GloveConfig.builder()
                                       .name("primary")
                                       .side(side)
                                       .connection(port == null ? SerialConnection.autoDiscover()
                                                                : SerialConnection.port(port))
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      GloveDeviceInfo info = device.getDeviceInfo();
      System.out.println("source=" + device.sourceId()
          + " canonical_sn=" + info.canonicalSn
          + " device_type=" + (info.deviceType == null ? "unknown" : info.deviceType)
          + " model=" + (info.model == null ? "unknown" : info.model)
          + " firmware_version=" + (info.firmwareVersion.isEmpty() ? "unknown" : info.firmwareVersion)
          + " hand=" + info.hand.wireValue()
          + " hand_source=" + (info.handSource == null ? "unknown" : info.handSource)
          + " transport_type=" + info.transportType.name().toLowerCase(Locale.ROOT));
    }
  }
}
