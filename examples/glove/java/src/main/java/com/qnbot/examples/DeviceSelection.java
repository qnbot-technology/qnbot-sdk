package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.error.AmbiguousSelectionException;
import com.qnbot.sdk.glove.ExternalConnection;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;

/**
 * Resolves gloves by name and side, then shows the ambiguity errors for unqualified selectors.
 *
 * <p>No Android transport is needed: the devices use external connections. Supply the Android
 * Context; the SDK closes after the three selections and two expected ambiguity checks.
 */
public final class DeviceSelection {
  private DeviceSelection() {}

  public static void run(Context context) throws Exception {
    SdkConfig config = SdkConfig.builder()
                           .device(GloveConfig.builder()
                                       .name("primary")
                                       .side(Side.LEFT)
                                       .connection(ExternalConnection.INSTANCE)
                                       .build())
                           .device(GloveConfig.builder()
                                       .name("primary")
                                       .side(Side.RIGHT)
                                       .connection(ExternalConnection.INSTANCE)
                                       .build())
                           .device(GloveConfig.builder()
                                       .name("backup")
                                       .side(Side.LEFT)
                                       .connection(ExternalConnection.INSTANCE)
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context); Glove glove = sdk.glove()) {
      System.out.println("primary left source_id=" + glove.device("primary", Side.LEFT).sourceId());
      System.out.println(
          "primary right source_id=" + glove.device("primary", Side.RIGHT).sourceId());
      System.out.println("backup left source_id=" + glove.device("backup").sourceId());
      try {
        glove.device("primary");
        throw new IllegalStateException("name=primary should match two devices");
      } catch (AmbiguousSelectionException expected) {
        System.out.println("expected ambiguity: name=primary");
      }
      try {
        glove.device(Side.LEFT);
        throw new IllegalStateException("side=left should match two devices");
      } catch (AmbiguousSelectionException expected) {
        System.out.println("expected ambiguity: side=left");
      }
    }
    System.out.println("device selection complete");
  }
}
