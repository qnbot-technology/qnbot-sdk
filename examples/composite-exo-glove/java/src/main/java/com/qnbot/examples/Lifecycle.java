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
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;

/**
 * Runs one bounded composite lifecycle: start the root Sdk, advance one update, print the member
 * sources and the latest glove pose and Exo telemetry samples, then stop and close. Callers supply
 * an authorized transport and the shared port; the method returns after cleanup.
 */
public final class Lifecycle {
  private Lifecycle() {}

  public static void run(Context context, SerialTransport transport, String port) {
    SdkConfig config = SdkConfig.builder()
        .device(CompositeExoGloveConfig.builder()
            .connection(SerialConnection.port(port))
            .devices(GloveConfig.builder().side(Side.LEFT).build(),
                GloveConfig.builder().side(Side.RIGHT).build(), ExoConfig.builder().build())
            .build())
        .build();
    try (Sdk sdk = new Sdk(config, context, transport)) {
      // One root Sdk owns the shared link and both member domains.
      Glove glove = sdk.glove();
      Exo exo = sdk.exo();
      GloveDevice left = glove.device(Side.LEFT);
      GloveDevice right = glove.device(Side.RIGHT);
      ExoDevice exoDevice = exo.device();
      sdk.start();
      sdk.update();
      System.out.println("started Left Glove=" + left.sourceId()
          + " Right Glove=" + right.sourceId() + " Exo=" + exoDevice.sourceId());
      left.pose().latest().ifPresent(
          sample -> System.out.println("left sequence=" + sample.sequence));
      right.pose().latest().ifPresent(
          sample -> System.out.println("right sequence=" + sample.sequence));
      exoDevice.telemetry().latest().ifPresent(
          sample -> System.out.println("Exo sequence=" + sample.sequence));
      sdk.stop();
      System.out.println("stopped");
    }
    System.out.println("closed");
  }
}
