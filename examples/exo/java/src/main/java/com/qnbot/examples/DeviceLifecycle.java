package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.ExoSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.exo.Exo;
import com.qnbot.sdk.exo.ExoConfig;
import com.qnbot.sdk.exo.ExoDevice;

/** DeviceLifecycle: the SDK workflow, invoked by an existing Android consumer. */
public final class DeviceLifecycle {
  private DeviceLifecycle() {}

  public static void run(Context context, SerialTransport transport, String firstPort,
      String secondPort) throws Exception {
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
    try (ExoSdk sdk = new ExoSdk(config, context, transport); Exo exo = sdk.exo()) {
      ExoDevice first = exo.device("first");
      ExoDevice second = exo.device("second");
      first.telemetry().subscribe(
          sample -> System.out.println("first sequence=" + sample.sequence));
      second.telemetry().subscribe(
          sample -> System.out.println("second sequence=" + sample.sequence));
      first.start();
      second.start();
      exo.runBackground();
      try {
        Thread.sleep(3000);
        second.stop();
        System.out.println("second stopped; first continues");
        Thread.sleep(3000);
        second.start();
        System.out.println("second restarted");
        Thread.sleep(3000);
        second.stop();
        second.close();
        System.out.println("second closed; first continues");
        Thread.sleep(3000);
      } finally {
        exo.requestStop();
        exo.join();
      }
    }
  }
}
