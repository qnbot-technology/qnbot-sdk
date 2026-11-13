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

/** Streams raw EMF observations in signed demodulated-integral matrices. */
public final class Emf {
  private Emf() {}

  public static void run(Context context, SerialTransport transport, String port, Side side) {
    SdkConfig config = SdkConfig.builder()
                           .device(GloveConfig.builder()
                                       .name("primary")
                                       .side(side)
                                       .connection(SerialConnection.port(port))
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      device.emf().subscribe(sample -> {
        com.qnbot.sdk.glove.GloveEmfPayload payload = sample.value.payload;
        for (com.qnbot.sdk.glove.GloveEmfChannel channel : payload.channels) {
          System.out.println("EMF session=" + payload.session
              + " frame_sequence=" + payload.frameSequence + " sync=" + payload.sync
              + " channel=" + channel.channelId + " units=" + payload.units + " li=" + channel.li
              + " lq=" + channel.lq + " hi=" + channel.hi + " hq=" + channel.hq);
        }
      });
      glove.start();
      glove.runForever();
    }
  }
}
