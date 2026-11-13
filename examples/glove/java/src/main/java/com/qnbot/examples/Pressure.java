package com.qnbot.examples;

import android.content.Context;
import com.qnbot.sdk.Sdk;
import com.qnbot.sdk.SdkConfig;
import com.qnbot.sdk.android.SerialTransport;
import com.qnbot.sdk.channel.ReadChannel;
import com.qnbot.sdk.channel.Sample;
import com.qnbot.sdk.config.SerialConnection;
import com.qnbot.sdk.glove.Glove;
import com.qnbot.sdk.glove.GloveConfig;
import com.qnbot.sdk.glove.GloveDevice;
import com.qnbot.sdk.glove.GlovePressure;
import java.util.List;
import java.util.Locale;

/**
 * Reads full-frame raw pressure summaries and one point's kPa estimate.
 *
 * <p>Pass the host Context, authorized serial transport and glove port. Run on a worker thread.
 * The optional point index is 0..459; null selects the peak. The first-frame window is five
 * seconds, followed by continuous foreground execution with at most one displayed frame per second.
 */
public final class Pressure {
  private Pressure() {}

  public static void run(Context context, SerialTransport transport, String port) {
    run(context, transport, port, null);
  }

  public static void run(Context context, SerialTransport transport, String port, Integer point) {
    if (point != null && (point < 0 || point >= 460))
      throw new IllegalArgumentException("point must be between 0 and 459");
    SdkConfig config =
        SdkConfig.builder()
            .device(GloveConfig.builder().connection(SerialConnection.port(port)).build())
            .build();
    try (Sdk sdk = new Sdk(config, context, transport); Glove glove = sdk.glove()) {
      GloveDevice device = glove.device();
      ReadChannel<GlovePressure> pressure = device.pressure();
      pressure.withConfig(com.qnbot.sdk.channel.SubscriptionConfig.latest(1.0))
          .subscribe(sample -> printPressure(sample, point));
      glove.start();
      System.out.println("waiting for pressure (5 s); kPa is an estimate capped at ~40.01, "
          + "not force");
      long deadline = System.nanoTime() + 5_000_000_000L;
      while (!pressure.latest().isPresent()) {
        if (System.nanoTime() >= deadline)
          throw new IllegalStateException("No pressure frame within 5 s; check the HMP-MX model, "
              + "serial port and device reporting.");
        glove.update();
        try {
          Thread.sleep(10);
        } catch (InterruptedException interrupted) {
          Thread.currentThread().interrupt();
          return;
        }
      }
      glove.runForever();
    }
  }

  private static void printPressure(Sample<GlovePressure> sample, Integer point) {
    List<Integer> readings = sample.value.readings;
    int minimum = 255;
    int maximum = -1;
    int peak = 0;
    int total = 0;
    int capped = 0;
    for (int i = 0; i < readings.size(); i++) {
      int raw = readings.get(i);
      minimum = Math.min(minimum, raw);
      total += raw;
      if (raw > maximum) {
        maximum = raw;
        peak = i;
      }
      if (raw >= 107)
        capped++;
    }
    int selected = point == null ? peak : point;
    System.out.printf(Locale.ROOT,
        "pressure sequence=%d points=%d raw_min=%d raw_mean=%.1f raw_max=%d "
            + "peak_index=%d capped_points=%d point_index=%d "
            + "point_row=%d point_column=%d point_raw=%d point_estimated_kpa=%.1f%n",
        sample.sequence, readings.size(), minimum, (double) total / readings.size(), maximum, peak,
        capped, selected, selected / 20, selected % 20, readings.get(selected),
        sample.value.estimatedKpa.get(selected));
  }
}
