package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.SleepResult;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.exo.ExoDevice;
import java.util.function.BooleanSupplier;

/**
 * Reads Exo telemetry on the caller's worker: configure, subscribe, start, then update.
 *
 * <p>Supply an Android {@link Context}, an authorized {@link SerialTransport} and a port; a null
 * port auto-discovers the only device. Return true from {@code cancelled} to stop; the SDK closes
 * when the update loop exits.
 */
public final class QuickStart {
  private QuickStart() {}

  public static void run(Context context, SerialTransport transport, String port,
      BooleanSupplier cancelled) throws InterruptedException {
    if (cancelled.getAsBoolean())
      return;
    SerialConnection connection =
        port == null ? SerialConnection.autoDiscover() : SerialConnection.port(port);
    SdkConfig config = SdkConfig.builder()
                           .device(ExoConfig.builder().connection(connection).build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport); Exo exo = sdk.exo()) {
      ExoDevice device = exo.device();
      // Subscribe before start so the channel is ready when the first sample arrives.
      device.telemetry().subscribe(sample
          -> System.out.println("telemetry sequence=" + sample.sequence
              + " leftEncoders=" + sample.value.payload.leftArm.encoderCounts
              + " rightEncoders=" + sample.value.payload.rightArm.encoderCounts
              + " leftRadians=" + sample.value.payload.leftArm.jointPositionsRad
              + " rightRadians=" + sample.value.payload.rightArm.jointPositionsRad));
      exo.start();
      // Advance SDK work on this thread; the caller's stop signal ends the loop.
      while (!cancelled.getAsBoolean()) {
        var update = exo.update();
        if (!update.hasNextTask) {
          Thread.sleep(20);
        } else if (update.sleep() == SleepResult.STOPPED) {
          break;
        }
      }
    }
  }
}
