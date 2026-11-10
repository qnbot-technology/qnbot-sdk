package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.exo.ExoDevice;

/**
 * Streams the automatically discovered Exo's telemetry.
 *
 * <p>Supply an Android {@link Context} and an authorized {@link SerialTransport}. Subscribe before
 * start so the channel is ready when the first sample arrives; the subscription prints each
 * sequence.
 */
public final class QuickStart {
  private QuickStart() {}

  public static void run(Context context, SerialTransport transport) {
    SdkConfig config = SdkConfig.builder().device(ExoConfig.builder().build()).build();
    try (Sdk sdk = new Sdk(config, context, transport); Exo exo = sdk.exo()) {
      ExoDevice device = exo.device();
      device.telemetry().subscribe(sample
          -> System.out.println("telemetry sequence=" + sample.sequence
              + " leftEncoders=" + sample.value.payload.leftArm.encoderCounts
              + " rightEncoders=" + sample.value.payload.rightArm.encoderCounts
              + " leftRadians=" + sample.value.payload.leftArm.jointPositionsRad
              + " rightRadians=" + sample.value.payload.rightArm.jointPositionsRad));
      exo.start();
      exo.runForever();
    }
  }
}
