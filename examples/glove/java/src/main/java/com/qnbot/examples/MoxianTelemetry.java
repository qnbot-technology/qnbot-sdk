package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.SleepResult;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.GloveFinger;
import java.util.function.BooleanSupplier;

/** Reads motion, wrist-local bone poses, and pressure from a Moxian glove. */
public final class MoxianTelemetry {
  private MoxianTelemetry() {}

  public static void run(Context context, SerialTransport transport, String port,
      BooleanSupplier cancelled) throws InterruptedException {
    if (cancelled.getAsBoolean()) {
      return;
    }
    SdkConfig config = SdkConfig.builder()
                           .device(GloveConfig.builder()
                                       .name("primary")
                                       .connection(SerialConnection.port(port))
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      var skeleton = device.skeleton();
      var display = com.qnbot.sdk.channel.SubscriptionConfig.latest(1.0);
      device.pose().withConfig(display).subscribe(sample -> {
        var tip = sample.value.payload.fingertipLocal.get(GloveFinger.INDEX);
        System.out.println("pose sequence=" + sample.sequence + " index_position=" + tip.position);
      });
      device.imu().withConfig(display).subscribe(sample ->
          System.out.println("imu sequence=" + sample.sequence
              + " gyroscope_raw=" + sample.value.payload.gyroscopeRaw
              + " accelerometer_raw=" + sample.value.payload.accelerometerRaw));
      skeleton.jointAngles().withConfig(display).subscribe(sample ->
          System.out.println("joint_angles sequence=" + sample.sequence + " joints="
              + sample.value.joints));
      skeleton.pose().withConfig(display).subscribe(sample -> {
        var pose = sample.value;
        int index = pose.names.indexOf("index_tip");
        System.out.println("skeleton_pose sequence=" + sample.sequence + " frame="
            + pose.coordinateFrame + " node=index_tip position=" + pose.positionsLocal.get(index)
            + " quaternion_xyzw=" + pose.orientationsLocalXyzw.get(index));
      });
      device.pressure().withConfig(display).subscribe(sample ->
          System.out.println("pressure sequence=" + sample.sequence + " points="
              + sample.value.readings.size() + " first_raw=" + sample.value.readings.get(0)));
      glove.start();
      while (!cancelled.getAsBoolean()) {
        SleepResult outcome = glove.update().sleep();
        if (outcome == SleepResult.STOPPED) {
          break;
        }
        if (outcome == SleepResult.NO_TASKS) {
          Thread.sleep(10);
        }
      }
    }
  }
}
