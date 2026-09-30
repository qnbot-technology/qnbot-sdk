from __future__ import annotations

import argparse
import time

from qnbot_sdk import CompositeExoGloveConfig, Sdk, SerialConnection, Side
from qnbot_sdk.exo import ExoConfig
from qnbot_sdk.glove import GloveConfig, GloveFinger, GloveHaptics


def main() -> None:
    arguments = argparse.ArgumentParser(description="Control Glove haptics in a composite")
    arguments.add_argument("--port", required=True, help="Shared serial port")
    arguments.add_argument("--hold", type=float, default=1.0)
    options = arguments.parse_args()
    if options.hold < 0:
        arguments.error("--hold must not be negative")

    sdk = Sdk(
        devices=(
            CompositeExoGloveConfig(
                connection=SerialConnection(port=options.port),
                devices=(
                    GloveConfig(side=Side.LEFT),
                    GloveConfig(side=Side.RIGHT),
                    ExoConfig(),
                ),
            ),
        )
    )
    glove = sdk.glove()
    left_device = glove.device(side=Side.LEFT)
    right_device = glove.device(side=Side.RIGHT)
    try:
        sdk.start()
        value = GloveHaptics({GloveFinger.THUMB: 80, GloveFinger.INDEX: 40})
        left = left_device.haptics()
        right = right_device.haptics()
        left.set(value)
        right.set(value)
        print("set Glove haptics on left and right Glove")
        time.sleep(options.hold)
        left.clear()
        right.clear()
        print("Glove haptics cleared")
    finally:
        sdk.stop()
        sdk.close()


if __name__ == "__main__":
    main()
