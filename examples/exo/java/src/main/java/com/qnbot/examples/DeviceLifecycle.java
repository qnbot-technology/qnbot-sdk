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
 * Runs two Exo devices independently: stop the second and keep reading the first.
 *
 * <p>Supply an Android {@link Context}, an authorized {@link SerialTransport} and two distinct
 * ports. The example waits {@code seconds}, stops the second device, reads the first device's
 * health and retained telemetry, then closes both.
 */
public final class DeviceLifecycle {
  private DeviceLifecycle() {}

  public static void run(Context context, SerialTransport transport, String firstPort,
      String secondPort, double seconds) throws Exception {
    if (!Double.isFinite(seconds) || seconds <= 0)
      throw new IllegalArgumentException("seconds must be positive and finite");
    if (firstPort.equals(secondPort))
      throw new IllegalArgumentException("Use two distinct Exo ports");
    SdkConfig config = SdkConfig.builder()
                           .device(ExoConfig.builder()
                                       .name("first")
                                       .connection(SerialConnection.port(firstPort))
                                       .build())
                           .device(ExoConfig.builder()
                                       .name("second")
                                       .connection(SerialConnection.port(secondPort))
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport); Exo exo = sdk.exo()) {
      ExoDevice first = exo.device("first");
      ExoDevice second = exo.device("second");
      first.telemetry().subscribe(
          sample -> System.out.println("first sequence=" + sample.sequence));
      second.telemetry().subscribe(
          sample -> System.out.println("second sequence=" + sample.sequence));
      first.start();
      second.start();
      System.out.println("first health=" + first.health() + " second health=" + second.health());
      Thread.sleep((long) (seconds * 1000));
      // Stopping one device leaves the other running and its retained sample readable.
      second.stop();
      System.out.println("second stopped; first continues");
      System.out.println("first health=" + first.health());
      first.telemetry().latest().ifPresent(
          sample -> System.out.println("first retained sequence=" + sample.sequence));
      first.stop();
      first.close();
      second.close();
    }
  }
}
