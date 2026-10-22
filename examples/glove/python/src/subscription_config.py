from __future__ import annotations

import argparse
import math
import time

from qnbot_sdk import Sample, Sdk, SerialConnection, Side, SubscriptionConfig
from qnbot_sdk.glove import GloveConfig, GlovePose


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Configure pose subscription delivery"
    )
    arguments.add_argument("--port", required=True)
    arguments.add_argument("--side", choices=("left", "right"), required=True)
    arguments.add_argument("--mode", choices=("latest", "fifo"), default="latest")
    arguments.add_argument("--capacity", type=int, default=128)
    arguments.add_argument("--max-rate-hz", type=float, default=30.0)
    arguments.add_argument("--seconds", type=float, default=10.0)
    options = arguments.parse_args()
    if not math.isfinite(options.seconds) or options.seconds <= 0:
        arguments.error("--seconds must be finite and greater than 0")
    config = (
        SubscriptionConfig.latest(max_rate_hz=options.max_rate_hz)
        if options.mode == "latest"
        else SubscriptionConfig.fifo(
            capacity=options.capacity, max_rate_hz=options.max_rate_hz
        )
    )
    sdk = Sdk(
        devices=(
            GloveConfig(
                side=Side(options.side), connection=SerialConnection(port=options.port)
            ),
        )
    )
    glove = sdk.glove()
    device = glove.device()
    count = 0
    first_at = 0.0
    last_at = 0.0
    last_sequence = 0

    def record_pose(sample: Sample[GlovePose]) -> None:
        nonlocal count, first_at, last_at, last_sequence
        last_at = time.perf_counter()
        if count == 0:
            first_at = last_at
        count += 1
        last_sequence = sample.sequence

    try:
        device.pose().with_config(config).subscribe(record_pose)
        glove.start()
        glove.run_background()
        print(f"mode={options.mode} max_rate_hz={options.max_rate_hz}", flush=True)
        time.sleep(options.seconds)
        glove.request_stop()
        glove.join()
    finally:
        glove.close()
    observed_hz = (count - 1) / (last_at - first_at) if count > 1 else 0.0
    print(
        f"samples={count} observed_hz={observed_hz:.2f} last_sequence={last_sequence}"
    )
    print("subscription complete")


if __name__ == "__main__":
    main()
