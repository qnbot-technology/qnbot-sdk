package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.GloveFinger;

/**
 * Streams the automatically discovered glove's pose.
 *
 * <p>Pass the host {@link Context} and an authorized {@link SerialTransport}. Run on a worker
 * thread; the pose subscription prints each sequence.
 */
public final class QuickStart {
  private QuickStart() {}

  public static void run(Context context, SerialTransport transport) {
    SdkConfig config = SdkConfig.builder().device(GloveConfig.builder().build()).build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      device.pose().subscribe(sample -> System.out.println("pose sequence=" + sample.sequence
              + " thumb=" + sample.value.payload.fingertipLocal.get(GloveFinger.THUMB).position
              + " index=" + sample.value.payload.fingertipLocal.get(GloveFinger.INDEX).position));
      glove.start();
      System.out.println("running");
      glove.runForever();
    }
  }
}
