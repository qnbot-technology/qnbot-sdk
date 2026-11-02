package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.ExoSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.exo.ExoDevice;

/** Telemetry: the SDK workflow, invoked by an existing Android consumer. */
public final class Telemetry {
  private Telemetry() {}

  public static void run(Context context, SerialTransport transport, String port) throws Exception {
    SdkConfig config =
        SdkConfig.builder()
            .device(ExoConfig.builder().connection(SerialConnection.port(port)).build())
            .build();
    try (ExoSdk sdk = new ExoSdk(config, context, transport); Exo exo = sdk.exo()) {
      ExoDevice device = exo.device();
      device.telemetry().subscribe(sample
          -> System.out.println("telemetry sequence=" + sample.sequence
              + " leftEncoders=" + sample.value.payload.leftArm.encoderCounts
              + " rightEncoders=" + sample.value.payload.rightArm.encoderCounts
              + " leftRadians=" + sample.value.payload.leftArm.jointPositionsRad
              + " rightRadians=" + sample.value.payload.rightArm.jointPositionsRad));
      device.status().subscribe(sample
          -> System.out.println("status sequence=" + sample.sequence + " connected="
              + sample.value.connected + " stale=" + sample.value.stale + " reconnects="
              + sample.value.reconnectCount + " error=" + sample.value.lastError));
      exo.start();
      exo.runForever();
    }
  }
}
