package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.exo.DiscoveredExo;
import com.qnbot.sdk.exo.ExoDeviceInfo;
import java.io.IOException;
import java.util.List;

/**
 * Lists the Exo candidates reachable through the caller's Android serial transport.
 *
 * <p>Supply an Android {@link Context} and an authorized {@link SerialTransport}. Each candidate
 * prints its port and device information, or the per-port discovery error that replaced them.
 */
public final class DiscoverExos {
  private DiscoverExos() {}

  public static void run(Context context, SerialTransport transport) throws IOException {
    List<DiscoveredExo> found = Sdk.discoverExos(context, transport);
    for (DiscoveredExo device : found) {
      if (device.error != null) {
        System.out.println("port=" + device.port + " error=" + device.error.code
            + " message=" + device.error.message);
        continue;
      }
      ExoDeviceInfo info = device.deviceInfo;
      System.out.println("transport=" + device.transportType + " port=" + device.port
          + " product=" + info.product + " serial=" + info.serialNumber);
    }
  }
}
