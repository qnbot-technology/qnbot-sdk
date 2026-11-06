package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.SleepResult;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.CompositeExoGloveConfig;
import com.qnbot.sdk.config.DeviceSelector;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.config.TargetAlgorithm;
import com.qnbot.sdk.config.TargetConfig;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import java.util.function.BooleanSupplier;

/**
 * Subscribes to both retargeted hand outputs of one composite Exo+Glove device. Callers supply an
 * authorized transport, the shared port, a target-hand algorithm package prepared for this app and
 * a stop flag; the root Sdk drives both targets and the update loop runs on their worker.
 */
public final class GloveRetargeting {
  private GloveRetargeting() {}

  public static void run(Context context, SerialTransport transport, String port, String packageId,
      BooleanSupplier cancelled) throws InterruptedException {
    if (cancelled.getAsBoolean())
      return;
    SdkConfig config = SdkConfig.builder()
        .device(CompositeExoGloveConfig.builder()
            .connection(SerialConnection.port(port))
            .devices(GloveConfig.builder().side(Side.LEFT).build(),
                GloveConfig.builder().side(Side.RIGHT).build(), ExoConfig.builder().build())
            .build())
        .target(TargetConfig.builder()
            .name("left-hand")
            .side(Side.LEFT)
            .source(new DeviceSelector("glove", null, Side.LEFT))
            .algorithm(new TargetAlgorithm(packageId))
            .build())
        .target(TargetConfig.builder()
            .name("right-hand")
            .side(Side.RIGHT)
            .source(new DeviceSelector("glove", null, Side.RIGHT))
            .algorithm(new TargetAlgorithm(packageId))
            .build())
        .build();
    try (Sdk sdk = new Sdk(config, context, transport)) {
      Glove glove = sdk.glove();
      GloveDevice left = glove.device(Side.LEFT);
      GloveDevice right = glove.device(Side.RIGHT);
      // Subscribe both target outputs before start; the SDK keeps its default Capture interaction.
      left.output("left-hand").subscribe(sample
          -> System.out.println("left retargeting sequence=" + sample.sequence
              + " target=" + sample.value.target + " joints=" + sample.value.joints));
      right.output("right-hand").subscribe(sample
          -> System.out.println("right retargeting sequence=" + sample.sequence
              + " target=" + sample.value.target + " joints=" + sample.value.joints));
      sdk.start();
      while (!cancelled.getAsBoolean()) {
        var update = sdk.update();
        if (!update.hasNextTask) {
          Thread.sleep(20);
        } else if (update.sleep() == SleepResult.STOPPED) {
          break;
        }
      }
    }
  }
}
