from __future__ import annotations

import argparse
import time

from qnbot_sdk import Sample, Sdk, SerialConnection, SubscriptionConfig
from qnbot_sdk.glove import GloveConfig, GlovePressure


def print_pressure(sample: Sample[GlovePressure], point: int | None = None) -> None:
    readings = sample.value.readings
    maximum = max(readings)
    peak = readings.index(maximum)
    selected = peak if point is None else point
    print(
        f"pressure sequence={sample.sequence} points={len(readings)} "
        f"raw_min={min(readings)} raw_mean={sum(readings) / len(readings):.1f} "
        f"raw_max={maximum} peak_index={peak} "
        f"capped_points={sum(raw >= 107 for raw in readings)} "
        f"point_index={selected} point_row={selected // 20} point_column={selected % 20} "
        f"point_raw={readings[selected]} "
        f"point_estimated_kpa={sample.value.estimated_kpa[selected]:.1f}",
        flush=True,
    )


def main() -> None:
    arguments = argparse.ArgumentParser(description="Read glove pressure")
    arguments.add_argument("--port", required=True, help="Glove serial port")
    arguments.add_argument(
        "--point", type=int, help="Point index 0..459; default: peak"
    )
    options = arguments.parse_args()
    if options.point is not None and not 0 <= options.point < 460:
        arguments.error("--point must be between 0 and 459")
    sdk = Sdk(devices=(GloveConfig(connection=SerialConnection(port=options.port)),))
    glove = sdk.glove()
    device = glove.device()
    pressure = device.pressure()
    pressure.with_config(SubscriptionConfig.latest(max_rate_hz=1)).subscribe(
        lambda sample: print_pressure(sample, options.point)
    )

    try:
        glove.start()
        print(
            "waiting for pressure (5 s); kPa is an estimate capped at ~40.01, "
            "not force; press Ctrl+C to stop",
            flush=True,
        )
        deadline = time.monotonic() + 5.0
        while pressure.latest() is None:
            if time.monotonic() >= deadline:
                raise RuntimeError(
                    "No pressure frame within 5 s; check the HMP-MX model, "
                    "serial port and device reporting."
                )
            glove.update()
            time.sleep(0.01)
        glove.run_forever()
    finally:
        glove.close()


if __name__ == "__main__":
    main()
