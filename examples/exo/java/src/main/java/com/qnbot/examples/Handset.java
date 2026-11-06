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
 * Prints Exo handset axes, triggers and buttons while updating on the caller's worker.
 *
 * <p>Supply an Android {@link Context}, an authorized {@link SerialTransport} and the device
 * port; the device must report handset capability. Return true from {@code cancelled} to stop.
 */
public final class Handset {
  private Handset() {}

  public static void run(Context context, SerialTransport transport, String port,
      BooleanSupplier cancelled) throws InterruptedException {
    if (cancelled.getAsBoolean())
      return;
    SdkConfig config = SdkConfig.builder()
                           .device(ExoConfig.builder()
                                       .connection(SerialConnection.port(port))
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport); Exo exo = sdk.exo()) {
      ExoDevice device = exo.device();
      if (!device.getDeviceInfo().capabilities.handsets) {
        System.out.println("Device has no handset capability");
        return;
      }
      // Subscribe before start so the channel is ready when the first sample arrives.
      device.telemetry().subscribe(sample
          -> System.out.println("handsets sequence=" + sample.sequence
              + " left=" + sample.value.payload.leftHandset.axisXRaw + ","
              + sample.value.payload.leftHandset.axisYRaw
              + " right=" + sample.value.payload.rightHandset.axisXRaw + ","
              + sample.value.payload.rightHandset.axisYRaw
              + " leftTrigger=" + sample.value.payload.leftHandset.triggerRaw
              + " rightTrigger=" + sample.value.payload.rightHandset.triggerRaw
              + " leftButtons=" + sample.value.payload.leftHandset.buttonMask
              + " rightButtons=" + sample.value.payload.rightHandset.buttonMask));
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
