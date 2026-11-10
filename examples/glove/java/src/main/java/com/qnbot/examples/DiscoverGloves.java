package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.glove.DiscoveredGlove;
import java.io.IOException;
import java.util.List;

/**
 * Lists glove candidates reachable through the caller's authorized Android serial transport.
 *
 * <p>Supply an Android {@link Context} and an authorized {@link SerialTransport}; each candidate
 * prints its transport, port, hand, serial number and any per-port discovery error. The temporary
 * probe connections close before this method returns.
 */
public final class DiscoverGloves {
  private DiscoverGloves() {}

  public static void run(Context context, SerialTransport transport) throws IOException {
    List<DiscoveredGlove> found = Sdk.discoverGloves(context, transport);
    for (DiscoveredGlove device : found) {
      String information = "transport=" + device.transportType + " port=" + device.port
          + " hand=" + device.hand + " serial=" + device.sn;
      if (device.error == null) {
        System.out.println(information);
      } else {
        System.out.println(information + " error=" + device.error.code
            + " message=" + device.error.message);
      }
    }
  }
}
