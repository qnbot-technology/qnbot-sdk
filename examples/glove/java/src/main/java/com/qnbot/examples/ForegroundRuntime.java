package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.SleepResult;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.DeviceSelector;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.config.TargetAlgorithm;
import com.qnbot.sdk.config.TargetConfig;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import java.util.function.BooleanSupplier;

/**
 * Streams one retargeted hand output on the caller's thread until stopped.
 *
 * <p>Pass the host {@link Context}, an authorized transport, the glove port and side, the installed
 * algorithm package, and a non-blocking stop signal. Run on a worker thread; the target
 * subscription prints each command.
 */
public final class ForegroundRuntime {
  private ForegroundRuntime() {}

  public static void run(Context context, SerialTransport transport, String port, Side side,
      String packageId, BooleanSupplier cancelled) throws InterruptedException {
    if (cancelled.getAsBoolean())
      return;
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
      device.output("openxr_hand")
          .subscribe(sample
              -> System.out.println("retargeting sequence=" + sample.sequence
                  + " target=" + sample.value.target + " joints=" + sample.value.joints));
      glove.start();
      while (!cancelled.getAsBoolean()) {
        var update = glove.update();
        if (!update.hasNextTask) {
          Thread.sleep(20);
        } else if (update.sleep() == SleepResult.STOPPED) {
          break;
        }
      }
    }
  }
}
