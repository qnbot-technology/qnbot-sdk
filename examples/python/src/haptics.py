from __future__ import annotations

import argparse
import time

from qnbot_sdk import (
    Sample,
    Sdk,
    SerialConnection,
    Side,
)
from qnbot_sdk.glove import GloveConfig, GloveFinger, Haptics


def create_sdk(port: str, side: Side) -> Sdk:
    return Sdk(
        devices=(
            GloveConfig(
                side=side,
                connection=SerialConnection(port=port),
            ),
        )
    )


def print_haptics(sample: Sample[Haptics]) -> None:
    print(f"latest haptics sequence={sample.sequence} value={sample.value}")


def main() -> None:
    arguments = argparse.ArgumentParser(description="Control glove haptic channels")
    arguments.add_argument("--port", required=True, help="Serial port for the glove")
    arguments.add_argument(
        "--side",
        choices=(Side.LEFT.value, Side.RIGHT.value),
        required=True,
        help="Physical glove side",
    )
    arguments.add_argument("--hold", type=float, default=1.0)
    options = arguments.parse_args()
    if options.hold < 0:
        arguments.error("--hold must not be negative")

    sdk = create_sdk(options.port, Side(options.side))
    glove = sdk.glove()
    haptics = None
    try:
        glove.connect()
        glove.start()

        haptics = glove.device().haptics()
        haptics.set(
            Haptics(
                {
                    GloveFinger.THUMB: 80,
                    GloveFinger.INDEX: 40,
                }
            )
        )
        latest = haptics.latest()
        if latest is not None:
            print_haptics(latest)
        time.sleep(options.hold)
    finally:
        try:
            if haptics is not None:
                haptics.clear()
                print("haptics cleared")
        finally:
            glove.close()


if __name__ == "__main__":
    main()
