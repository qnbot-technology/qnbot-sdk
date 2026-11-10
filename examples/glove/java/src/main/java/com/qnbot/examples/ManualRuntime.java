package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.SleepResult;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.channel.ReadChannel;
import com.qnbot.sdk.config.DeviceSelector;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.config.TargetAlgorithm;
import com.qnbot.sdk.config.TargetConfig;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.HandJointCommand;

/**
 * Advances Glove work manually for a requested number of updates.
 *
 * <p>Pass the host {@link Context}, an authorized transport, the glove port and side, the installed
 * algorithm package, and a positive update count. Each step waits for scheduled work, yields
 * briefly when the SDK has none, then prints the latest retargeted output.
 */
public final class ManualRuntime {
  private ManualRuntime() {}

  public static void run(Context context, SerialTransport transport, String port, Side side,
      String packageId, int updates) throws InterruptedException {
    if (updates <= 0)
      throw new IllegalArgumentException("updates must be positive");
    SdkConfig config = SdkConfig.builder()
        .device(GloveConfig.builder()
                    .name("primary")
                    .side(side)
                    .connection(SerialConnection.port(port))
                    .build())
        .target(TargetConfig.builder()
                    .name("openxr_hand")
                    .side(side)
                    .source(new DeviceSelector("glove", "primary", side))
                    .algorithm(new TargetAlgorithm(packageId))
                    .build())
        .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      ReadChannel<HandJointCommand> output = device.output("openxr_hand");
      glove.start();
      for (int step = 0; step < updates; step++) {
        SleepResult outcome = glove.update().sleep();
        if (outcome == SleepResult.STOPPED)
          break;
        if (outcome == SleepResult.NO_TASKS) {
          // Preparation or Capture still needs the caller; yield instead of spinning.
          Thread.sleep(10);
        }
        output.latest().ifPresent(sample
            -> System.out.println("retargeting sequence=" + sample.sequence
                + " target=" + sample.value.target + " joints=" + sample.value.joints));
      }
    }
  }
}
