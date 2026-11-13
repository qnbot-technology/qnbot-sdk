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
import com.qnbot.sdk.glove.GloveFinger;

/**
 * Reads combined telemetry from one composite Exo+Glove link: both glove poses, both glove status
 * streams, Exo telemetry and Exo status.
 *
 * <p>Callers supply the authorized transport and the shared port. The root Sdk drives all three
 * members.
 */
public final class Telemetry {
  private Telemetry() {}

  public static void run(Context context, SerialTransport transport, String port) {
    SdkConfig config = SdkConfig.builder()
                           .device(CompositeExoGloveConfig.builder()
                                       .connection(SerialConnection.port(port))
                                       .devices(GloveConfig.builder().side(Side.LEFT).build(),
                                           GloveConfig.builder().side(Side.RIGHT).build(),
                                           ExoConfig.builder().build())
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport)) {
      // One root Sdk exposes both member domains for the same composite link.
      Glove glove = sdk.glove();
      Exo exo = sdk.exo();
      GloveDevice left = glove.device(Side.LEFT);
      GloveDevice right = glove.device(Side.RIGHT);
      ExoDevice exoDevice = exo.device();
      left.pose().subscribe(sample
          -> System.out.println("left sequence=" + sample.sequence + " thumb="
              + (sample.value.payload.hand == null
                          || !sample.value.payload.hand.fingertips.containsKey(GloveFinger.THUMB)
                      ? "absent"
                      : sample.value.payload.hand.fingertips.get(GloveFinger.THUMB).position)
              + " index="
              + (sample.value.payload.hand == null
                          || !sample.value.payload.hand.fingertips.containsKey(GloveFinger.INDEX)
                      ? "absent"
                      : sample.value.payload.hand.fingertips.get(GloveFinger.INDEX).position)));
      right.pose().subscribe(sample
          -> System.out.println("right sequence=" + sample.sequence + " thumb="
              + (sample.value.payload.hand == null
                          || !sample.value.payload.hand.fingertips.containsKey(GloveFinger.THUMB)
                      ? "absent"
                      : sample.value.payload.hand.fingertips.get(GloveFinger.THUMB).position)
              + " index="
              + (sample.value.payload.hand == null
                          || !sample.value.payload.hand.fingertips.containsKey(GloveFinger.INDEX)
                      ? "absent"
                      : sample.value.payload.hand.fingertips.get(GloveFinger.INDEX).position)));
      exoDevice.telemetry().subscribe(sample
          -> System.out.println("telemetry sequence=" + sample.sequence
              + " leftEncoders=" + sample.value.payload.leftArm.encoderCounts
              + " rightEncoders=" + sample.value.payload.rightArm.encoderCounts
              + " leftRadians=" + sample.value.payload.leftArm.jointPositionsRad
              + " rightRadians=" + sample.value.payload.rightArm.jointPositionsRad));
      left.status().subscribe(sample
          -> System.out.println(
              "left connected=" + sample.value.connected + " stale=" + sample.value.stale));
      right.status().subscribe(sample
          -> System.out.println(
              "right connected=" + sample.value.connected + " stale=" + sample.value.stale));
      exoDevice.status().subscribe(sample
          -> System.out.println(
              "exo connected=" + sample.value.connected + " stale=" + sample.value.stale));
      // Start after subscribing, then let the root runner drive every scheduled task.
      sdk.start();
      sdk.runForever();
    }
  }
}
