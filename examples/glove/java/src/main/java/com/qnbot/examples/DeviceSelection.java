package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.GloveSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.error.AmbiguousSelectionException;
import com.qnbot.sdk.glove.ExternalConnection;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;

/** DeviceSelection: the SDK workflow, invoked by an existing Android consumer. */
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
    try (GloveSdk sdk = new GloveSdk(config, context); Glove glove = sdk.glove()) {
      System.out.println("primary left source_id=" + glove.device("primary", Side.LEFT).sourceId());
      System.out.println(
          "primary right source_id=" + glove.device("primary", Side.RIGHT).sourceId());
      System.out.println("backup left source_id=" + glove.device("backup").sourceId());
      try {
        glove.device("primary");
        throw new IllegalStateException("primary 应产生歧义");
      } catch (AmbiguousSelectionException expected) {
        System.out.println("expected ambiguity: name=primary");
      }
      try {
        glove.device(Side.LEFT);
        throw new IllegalStateException("left 应产生歧义");
      } catch (AmbiguousSelectionException expected) {
        System.out.println("expected ambiguity: side=left");
      }
    }
    System.out.println("device selection complete");
  }
}
