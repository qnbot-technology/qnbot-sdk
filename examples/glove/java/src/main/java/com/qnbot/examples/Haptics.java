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
import com.qnbot.sdk.glove.GloveFinger;

/**
 * Sets thumb and index haptics, holds them, then clears every channel.
 *
 * <p>Pass the host {@link Context}, an authorized transport, the glove port and side, and the
 * nonnegative hold time. The latest retained haptics sample is printed after the set; cleanup
 * always clears the channels.
 */
public final class Haptics {
  private Haptics() {}

  public static void run(Context context, SerialTransport transport, String port, Side side,
      double holdSeconds) throws InterruptedException {
    if (!Double.isFinite(holdSeconds) || holdSeconds < 0)
      throw new IllegalArgumentException("holdSeconds must be finite and nonnegative");
    SdkConfig config = SdkConfig.builder()
        .device(GloveConfig.builder()
                    .name("primary")
                    .side(side)
                    .connection(SerialConnection.port(port))
                    .build())
        .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      glove.start();
      try {
        device.haptics().set(com.qnbot.sdk.glove.Haptics.builder()
                                 .set(GloveFinger.THUMB, 80)
                                 .set(GloveFinger.INDEX, 40)
                                 .build());
        device.haptics().latest().ifPresent(sample
            -> System.out.println("latest haptics sequence=" + sample.sequence
                + " value=" + sample.value.strengths));
        Thread.sleep((long) (holdSeconds * 1_000));
      } finally {
        device.haptics().clear();
      }
    }
  }
}
