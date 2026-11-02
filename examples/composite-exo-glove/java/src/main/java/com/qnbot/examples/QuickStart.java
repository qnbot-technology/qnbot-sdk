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

/** QuickStart: the SDK workflow, invoked by an existing Android consumer. */
public final class QuickStart {
  private QuickStart() {}

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
      left.pose().subscribe(sample
          -> System.out.println("left sequence=" + sample.sequence
              + " thumb=" + sample.value.payload.fingertipLocal.get(GloveFinger.THUMB).position
              + " index=" + sample.value.payload.fingertipLocal.get(GloveFinger.INDEX).position));
      right.pose().subscribe(sample
          -> System.out.println("right sequence=" + sample.sequence
              + " thumb=" + sample.value.payload.fingertipLocal.get(GloveFinger.THUMB).position
              + " index=" + sample.value.payload.fingertipLocal.get(GloveFinger.INDEX).position));
      sdk.start();
      sdk.runForever();
    }
  }
}
