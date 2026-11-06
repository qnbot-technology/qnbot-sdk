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
 * Runs retargeting on the SDK background thread for a fixed duration.
 *
 * <p>Pass the host {@link Context}, an authorized transport, the glove port and side, the installed
 * algorithm package, and the positive run time. The target subscription prints commands; cleanup
 * requests stop and joins the runner.
 */
public final class BackgroundRuntime {
  private BackgroundRuntime() {}

  public static void run(Context context, SerialTransport transport, String port, Side side,
      String packageId, double seconds) throws InterruptedException {
    if (!Double.isFinite(seconds) || seconds <= 0)
      throw new IllegalArgumentException("seconds must be positive and finite");
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
      glove.runBackground();
      try {
        Thread.sleep((long) (seconds * 1_000));
      } finally {
        glove.requestStop();
        glove.join();
      }
    }
    System.out.println("background stopped");
  }
}
