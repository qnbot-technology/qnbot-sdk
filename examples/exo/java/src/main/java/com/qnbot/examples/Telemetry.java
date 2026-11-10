package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.exo.ExoDevice;

/**
 * Subscribes to Exo telemetry and connection status.
 *
 * <p>Supply an Android {@link Context}, an authorized {@link SerialTransport} and the device port.
 */
public final class Telemetry {
  private Telemetry() {}

  public static void run(Context context, SerialTransport transport, String port) {
    SdkConfig config = SdkConfig.builder()
                           .device(ExoConfig.builder()
                                       .connection(SerialConnection.port(port))
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport); Exo exo = sdk.exo()) {
      ExoDevice device = exo.device();
      // Subscribe before start so both channels are ready when the first sample arrives.
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
