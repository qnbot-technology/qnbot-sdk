package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.CompositeExoGloveConfig;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.GloveFinger;
import com.qnbot.sdk.glove.GloveHandPose;

/**
 * Connects one composite Exo+Glove device and prints the pose of both gloves.
 *
 * <p>Supply an Android {@link Context} and an authorized {@link SerialTransport}; the SDK
 * auto-discovers the shared endpoint. One root Sdk owns the whole composite link.
 */
public final class QuickStart {
  private QuickStart() {}

  public static void run(Context context, SerialTransport transport) {
    SdkConfig config = SdkConfig.builder()
                           .device(CompositeExoGloveConfig.builder()
                                       .devices(GloveConfig.builder().side(Side.LEFT).build(),
                                           GloveConfig.builder().side(Side.RIGHT).build(),
                                           ExoConfig.builder().build())
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport)) {
      Glove glove = sdk.glove();
      GloveDevice left = glove.device(Side.LEFT);
      GloveDevice right = glove.device(Side.RIGHT);
      // Subscribe before start so the first samples of both gloves are observed.
      left.pose().subscribe(
          sample -> printPose("left", sample.sequence, sample.value.payload.hand));
      right.pose().subscribe(
          sample -> printPose("right", sample.sequence, sample.value.payload.hand));
      // The root Sdk starts both glove members and the Exo over their shared serial link.
      sdk.start();
      sdk.runForever();
    }
  }

  private static void printPose(String label, long sequence, GloveHandPose hand) {
    System.out.println(label + " sequence=" + sequence
        + " thumb=" + hand.fingertips.get(GloveFinger.THUMB).position
        + " index=" + hand.fingertips.get(GloveFinger.INDEX).position);
  }
}
