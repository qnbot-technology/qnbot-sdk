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
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.GloveFinger;
import com.qnbot.sdk.glove.Haptics;

/** GloveHaptics: the SDK workflow, invoked by an existing Android consumer. */
public final class GloveHaptics {
  private GloveHaptics() {}

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
      sdk.start();
      Haptics request =
          Haptics.builder().set(GloveFinger.THUMB, 80).set(GloveFinger.INDEX, 40).build();
      try {
        left.haptics().set(request);
        right.haptics().set(request);
        System.out.println("Composite Glove haptics requests are ignored; no physical output");
        Thread.sleep(1000);
      } finally {
        left.haptics().clear();
        right.haptics().clear();
      }
    }
  }
}
