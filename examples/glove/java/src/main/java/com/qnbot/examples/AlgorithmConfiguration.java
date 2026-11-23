package com.qnbot.examples;

import android.content.Context;
import android.net.Uri;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.algorithm.AlgorithmPackageManager;
import com.qnbot.sdk.algorithm.InstallBatchResult;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.config.BoneConfigMode;
import com.qnbot.sdk.config.DeviceSelector;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.config.Side;
import com.qnbot.sdk.config.TargetAlgorithm;
import com.qnbot.sdk.config.TargetConfig;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import java.io.IOException;

/**
 * Prepares an algorithm package or bone configuration, then runs one configured target for a
 * fixed duration and prints retargeting output.
 *
 * <p>Supply the Android Context, authorized transport, explicit serial port, side and installed
 * package ID. Pass null algorithmConfigPath when the package uses its defaults; try-with-resources
 * closes the SDK after the timed run.
 */
public final class AlgorithmConfiguration {
  private AlgorithmConfiguration() {}

  /** Optional preparation before run(): import the Android algorithm ZIP selected by the host. */
  public static InstallBatchResult importAlgorithmArchive(Context context, Uri selectedZip)
      throws IOException {
    return new AlgorithmPackageManager(context, "glove").installFromArchive(selectedZip);
  }

  /**
   * Runs the configured target for {@code seconds} and prints retargeting output.
   */
  public static void run(Context context, SerialTransport transport, String port, Side side,
      String packageId, BoneConfigMode boneConfigMode, String algorithmConfigPath, double seconds)
      throws InterruptedException {
    if (!Double.isFinite(seconds) || seconds <= 0) {
      throw new IllegalArgumentException("seconds must be positive and finite");
    }
    SdkConfig config = SdkConfig.builder()
                           .device(GloveConfig.builder()
                                       .name("primary")
                                       .side(side)
                                       .connection(SerialConnection.port(port))
                                       .build())
                           .target(TargetConfig.builder()
                                       .name("configured_hand")
                                       .side(side)
                                       .source(new DeviceSelector("glove", "primary", side))
                                       .algorithm(new TargetAlgorithm(
                                           packageId, boneConfigMode, algorithmConfigPath))
                                       .build())
                           .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      device.output("configured_hand")
          .subscribe(sample
              -> System.out.println("retargeting sequence=" + sample.sequence
                  + " target=" + sample.value.target + " joints=" + sample.value.joints));
      glove.start();
      glove.runBackground();
      try {
        Thread.sleep((long) (seconds * 1_000));
      } finally {
        glove.requestStop();
        glove.join();
      }
    }
  }
}
