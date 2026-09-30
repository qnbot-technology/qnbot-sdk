from __future__ import annotations

import argparse
import time

from qnbot_sdk import (
    DeviceSelector,
    Sample,
    Sdk,
    SerialConnection,
    Side,
    TargetAlgorithm,
    TargetConfig,
)
from qnbot_sdk.glove import (
    GloveConfig,
    HandJointCommand,
)

def print_output(sample: Sample[HandJointCommand]) -> None:
    print(
        f"retargeting sequence={sample.sequence} "
        f"target={sample.value.target} joints={sample.value.joints}"
    )


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Run the Glove domain on its background thread"
    )
    arguments.add_argument("--port", required=True, help="Serial port for the glove")
    arguments.add_argument(
        "--side",
        choices=(Side.LEFT.value, Side.RIGHT.value),
        required=True,
        help="Physical glove side",
    )
    arguments.add_argument("--seconds", type=float, default=10.0)
    arguments.add_argument("--package-id", required=True)
    options = arguments.parse_args()
    if options.seconds <= 0:
        arguments.error("--seconds must be greater than 0")

    side = Side(options.side)
    source = DeviceSelector(type="glove", side=side)
    sdk = Sdk(
        devices=(
            GloveConfig(
                side=side,
                connection=SerialConnection(port=options.port),
            ),
        ),
        targets=(
            TargetConfig(
                name="openxr_hand",
                side=side,
                source=source,
                algorithms=(TargetAlgorithm(id=options.package_id),),
            ),
        ),
    )
    glove = sdk.glove()
    device = glove.device()
    try:
        device.output(name="openxr_hand").subscribe(print_output)
        glove.start()
        glove.run_background()
        time.sleep(options.seconds)
    finally:
        glove.request_stop()
        glove.join()
        glove.close()
    print("background stopped")


if __name__ == "__main__":
    main()
