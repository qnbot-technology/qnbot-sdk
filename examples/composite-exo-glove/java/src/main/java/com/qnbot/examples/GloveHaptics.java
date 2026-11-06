package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.CompositeExoGloveConfig;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.GloveFinger;
import com.qnbot.sdk.glove.Haptics;

/**
 * Submits haptics requests to both gloves of a composite Exo+Glove device and holds them for the
 * caller's duration. Composite devices accept but ignore glove haptics, so this example shows the
 * request path only. Callers supply the authorized transport, the shared port and hold seconds.
 */
public final class GloveHaptics {
  private GloveHaptics() {}

  public static void run(Context context, SerialTransport transport, String port,
      double holdSeconds) throws InterruptedException {
    if (!Double.isFinite(holdSeconds) || holdSeconds < 0)
      throw new IllegalArgumentException("holdSeconds must be finite and nonnegative");
    SdkConfig config = SdkConfig.builder()
        .device(CompositeExoGloveConfig.builder()
            .connection(SerialConnection.port(port))
            .devices(GloveConfig.builder().side(Side.LEFT).build(),
                GloveConfig.builder().side(Side.RIGHT).build(), ExoConfig.builder().build())
            .build())
        .build();
    try (Sdk sdk = new Sdk(config, context, transport)) {
      // One root Sdk owns the shared link; the glove domain selects both hands.
      Glove glove = sdk.glove();
      GloveDevice left = glove.device(Side.LEFT);
      GloveDevice right = glove.device(Side.RIGHT);
      sdk.start();
      Haptics request =
          Haptics.builder().set(GloveFinger.THUMB, 80).set(GloveFinger.INDEX, 40).build();
      try {
        left.haptics().set(request);
        right.haptics().set(request);
        System.out.println("Composite Glove haptics requests are ignored; no physical output");
        Thread.sleep((long) (holdSeconds * 1000));
      } finally {
        left.haptics().clear();
        right.haptics().clear();
      }
    }
  }
}
