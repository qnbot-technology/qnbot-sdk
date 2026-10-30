package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.ExoSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.exo.ExoDevice;

/** Handset: the SDK workflow, invoked by an existing Android consumer. */
public final class Handset {
  private Handset() {}

  public static void run(Context context, SerialTransport transport, String port) throws Exception {
    SdkConfig config =
        SdkConfig.builder()
            .device(ExoConfig.builder().connection(SerialConnection.port(port)).build())
            .build();
    try (ExoSdk sdk = new ExoSdk(config, context, transport); Exo exo = sdk.exo()) {
      ExoDevice device = exo.device();
      if (!device.getDeviceInfo().capabilities.handsets) {
        System.out.println("Device has no handset capability");
        return;
      }
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
      exo.runForever();
    }
  }
}
