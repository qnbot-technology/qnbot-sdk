package com.qnbot.examples;

import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.GloveHandPose;
import com.qnbot.sdk.glove.capture.CaptureQualityFeedback;
import com.qnbot.sdk.glove.capture.CaptureQualitySession;
import java.util.List;
import java.util.Locale;
import java.util.function.Supplier;

/** Assesses one fresh caller-collected window per stage. */
public final class CaptureQuality {
  private CaptureQuality() {}

  /** collectWindow returns a complete window after the caller settles each pose. */
  public static void run(GloveDevice device, String stage,
      Supplier<List<GloveHandPose>> collectWindow) {
    try (CaptureQualitySession detector = device.createCaptureQualitySession()) {
      System.out.println("required samples=" + detector.requiredSampleCount());
      for (String current : "close".equals(stage) ? List.of("close") : List.of("close", stage)) {
        System.out.println("摆好 " + current + " 并保持稳定，然后收集新窗口。");
        List<GloveHandPose> window = List.copyOf(collectWindow.get());
        CaptureQualityFeedback result = detector.assessStage(current, window);
        System.out.println("stage=" + current + " accepted=" + result.accepted + " action="
            + result.action.name().toLowerCase(Locale.ROOT) + " reason=" + result.reason
            + " reason_en=" + result.reasonEn);
        if (!result.accepted) break;
      }
    }
  }
}
