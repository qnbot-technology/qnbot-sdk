package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.SleepResult;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.channel.ReadChannel;
import com.qnbot.sdk.channel.Sample;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.GlovePose;
import java.util.Optional;

/**
 * Stops and restarts the right glove while the left keeps streaming pose samples.
 *
 * <p>Supply the Android Context, authorized transport, optional serial ports and the number of
 * fresh samples per phase. The example checks that a stopped device stops advancing.
 */
public final class DeviceLifecycle {
  private DeviceLifecycle() {}

  public static void run(Context context, SerialTransport transport, String leftPort,
      String rightPort, int updates) throws InterruptedException {
    if (updates <= 0) {
      throw new IllegalArgumentException("updates must be positive");
    }
    SdkConfig config = SdkConfig.builder()
                           .device(GloveConfig.builder()
                                       .name("left")
                                       .side(Side.LEFT)
                                       .connection(leftPort == null
                                               ? SerialConnection.autoDiscover()
                                               : SerialConnection.port(leftPort))
                                       .build())
                           .device(GloveConfig.builder()
                                       .name("right")
                                       .side(Side.RIGHT)
                                       .connection(rightPort == null
                                               ? SerialConnection.autoDiscover()
                                               : SerialConnection.port(rightPort))
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice left = glove.device("left");
      GloveDevice right = glove.device("right");
      ReadChannel<GlovePose> leftPose = left.pose();
      ReadChannel<GlovePose> rightPose = right.pose();
      left.start();
      right.start();
      System.out.println("both devices active");
      for (int phase = 0; phase < 3; phase++) {
        boolean rightActive = phase != 1;
        if (phase == 1) {
          right.stop();
          System.out.println("right device stopped; left remains active");
        } else if (phase == 2) {
          right.start();
          System.out.println("right device restarted");
        }
        long previousLeft = leftPose.latest().map(sample -> sample.sequence).orElse(-1L);
        long previousRight = rightPose.latest().map(sample -> sample.sequence).orElse(-1L);
        int leftCount = 0;
        int rightCount = 0;
        long deadline = System.nanoTime() + 10_000_000_000L;
        while (leftCount < updates || (rightActive && rightCount < updates)) {
          if (System.nanoTime() >= deadline) {
            throw new IllegalStateException("timed out waiting for pose samples");
          }
          var update = glove.update();
          if (update.sleep() == SleepResult.NO_TASKS) {
            Thread.sleep(10);
          }
          Optional<Sample<GlovePose>> leftSample = leftPose.latest();
          Optional<Sample<GlovePose>> rightSample = rightPose.latest();
          long rightSequence = rightSample.map(sample -> sample.sequence).orElse(-1L);
          if (!rightActive && rightSequence != previousRight) {
            throw new IllegalStateException("right pose advanced while stopped");
          }
          if (leftSample.isPresent() && leftSample.get().sequence != previousLeft) {
            previousLeft = leftSample.get().sequence;
            leftCount++;
            System.out.println("left pose sequence=" + leftSample.get().sequence);
          }
          if (rightActive && rightSample.isPresent() && rightSequence != previousRight) {
            previousRight = rightSequence;
            rightCount++;
            System.out.println("right pose sequence=" + rightSample.get().sequence);
          }
        }
      }
      System.out.println("lifecycle complete");
    }
  }
}
