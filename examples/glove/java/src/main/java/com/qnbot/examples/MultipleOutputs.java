package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.DeviceSelector;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.config.TargetAlgorithm;
import com.qnbot.sdk.config.TargetConfig;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;

/**
 * Reads two target outputs driven by one glove.
 *
 * <p>Pass the host {@link Context}, an authorized transport, the glove port and side, and the
 * installed algorithm package. Separate subscriptions label each output and close with the SDK.
 */
public final class MultipleOutputs {
  private MultipleOutputs() {}

  public static void run(Context context, SerialTransport transport, String port, Side side,
      String packageId) {
    SdkConfig config = SdkConfig.builder()
        .device(GloveConfig.builder()
                    .name("primary")
                    .side(side)
                    .connection(SerialConnection.port(port))
                    .build())
        .target(TargetConfig.builder()
                    .name("primary")
                    .side(side)
                    .source(new DeviceSelector("glove", "primary", side))
                    .algorithm(new TargetAlgorithm(packageId))
                    .build())
        .target(TargetConfig.builder()
                    .name("backup")
                    .side(side)
                    .source(new DeviceSelector("glove", "primary", side))
                    .algorithm(new TargetAlgorithm(packageId))
                    .build())
        .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      device.output("primary")
          .subscribe(sample
              -> System.out.println("callback primary sequence=" + sample.sequence
                  + " target=" + sample.value.target + " joints=" + sample.value.joints));
      device.output("backup")
          .subscribe(sample
              -> System.out.println("callback backup sequence=" + sample.sequence
                  + " target=" + sample.value.target + " joints=" + sample.value.joints));
      glove.start();
      glove.runForever();
    }
  }
}
