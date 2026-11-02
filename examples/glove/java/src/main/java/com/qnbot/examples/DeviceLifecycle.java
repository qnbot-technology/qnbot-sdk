package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.GloveSdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.SleepResult;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.channel.Sample;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.error.AmbiguousSelectionException;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.GlovePose;
import java.util.Optional;

/** DeviceLifecycle: the SDK workflow, invoked by an existing Android consumer. */
public final class DeviceLifecycle {
  private DeviceLifecycle() {}

  public static void run(Context context, SerialTransport transport, String leftPort,
      String rightPort) throws Exception {
    SdkConfig config = SdkConfig.builder()
                           .device(GloveConfig.builder()
                                       .name("left")
                                       .side(Side.LEFT)
                                       .connection(SerialConnection.port(leftPort))
                                       .build())
                           .device(GloveConfig.builder()
                                       .name("right")
                                       .side(Side.RIGHT)
                                       .connection(SerialConnection.port(rightPort))
                                       .build())
                           .build();
    try (GloveSdk sdk = new GloveSdk(config, context, transport); Glove glove = sdk.glove()) {
      try {
        glove.device();
        throw new IllegalStateException("未限定双手选择应产生歧义");
      } catch (AmbiguousSelectionException expected) {
        System.out.println("预期歧义：同时配置双手；使用名称或左右手选择");
      }
      System.out.println("left selected=" + glove.device("left", Side.LEFT).sourceId());
      System.out.println("right selected=" + glove.device(Side.RIGHT).sourceId());
      GloveDevice left = glove.device("left");
      GloveDevice right = glove.device("right");
      left.start();
      right.start();
      for (int phase = 0; phase < 3; phase++) {
        boolean rightActive = phase != 1;
        if (phase == 1) {
          right.stop();
          System.out.println("右手已停止；检查左手继续产生新帧");
        } else if (phase == 2) {
          right.start();
          System.out.println("右手重新启动");
        }
        long previousLeft = left.pose().latest().map(sample -> sample.sequence).orElse(-1L);
        long previousRight = right.pose().latest().map(sample -> sample.sequence).orElse(-1L);
        int leftCount = 0, rightCount = 0;
        long deadline = System.nanoTime() + 10_000_000_000L;
        while ((leftCount < 10 || (rightActive && rightCount < 10))) {
          if (System.nanoTime() >= deadline)
            throw new IllegalStateException("等待双手新帧超时");
          if (glove.update().sleep() == SleepResult.STOPPED)
            return;
          Optional<Sample<GlovePose>> l = left.pose().latest();
          Optional<Sample<GlovePose>> r = right.pose().latest();
          long rightSequence = r.map(sample -> sample.sequence).orElse(-1L);
          if (!rightActive && rightSequence != previousRight) {
            throw new IllegalStateException("右手停止后仍产生新姿态帧");
          }
          if (l.isPresent() && l.get().sequence != previousLeft) {
            previousLeft = l.get().sequence;
            leftCount++;
            System.out.println("left sequence=" + l.get().sequence);
          }
          if (rightActive && r.isPresent() && rightSequence != previousRight) {
            previousRight = rightSequence;
            rightCount++;
            System.out.println("right sequence=" + r.get().sequence);
          }
          Thread.sleep(10);
        }
      }
      System.out.println("双设备独立启停完成");
    }
  }
}
