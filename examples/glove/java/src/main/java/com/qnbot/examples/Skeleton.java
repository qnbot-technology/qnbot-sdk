package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.SleepResult;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import java.util.function.BooleanSupplier;

/**
 * Reads the built-in skeleton's joint angles and wrist-local pose.
 *
 * <p>Pass the host {@link Context}, an authorized transport, the glove port and side, and a
 * non-blocking stop signal. The skeleton is selected before start; its subscriptions print joint
 * commands and pose nodes until the caller stops.
 */
public final class Skeleton {
  private Skeleton() {}

  public static void run(Context context, SerialTransport transport, String port, Side side,
      BooleanSupplier cancelled) throws InterruptedException {
    if (cancelled.getAsBoolean())
      return;
    SdkConfig config = SdkConfig.builder()
        .device(GloveConfig.builder()
                    .name("primary")
                    .side(side)
                    .connection(SerialConnection.port(port))
                    .build())
        .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      com.qnbot.sdk.glove.Skeleton skeleton = device.skeleton();
      skeleton.jointAngles().subscribe(sample
          -> System.out.println("skeleton sequence=" + sample.sequence
              + " target=" + sample.value.target + " joints=" + sample.value.joints));
      skeleton.pose().subscribe(sample
          -> System.out.println("skeleton pose sequence=" + sample.sequence + " nodes="
              + sample.value.names.size() + " wrist=" + sample.value.positionsLocal.get(0)));
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
