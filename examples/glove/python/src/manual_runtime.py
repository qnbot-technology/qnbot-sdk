from __future__ import annotations

import argparse

from qnbot_sdk import (
    DeviceSelector,
    Sample,
    Sdk,
    SerialConnection,
    Side,
    TargetAlgorithm,
    TargetConfig,
)
from qnbot_sdk.glove import GloveConfig, HandJointCommand

def print_output(sample: Sample[HandJointCommand]) -> None:
    print(
        f"retargeting sequence={sample.sequence} "
        f"target={sample.value.target} joints={sample.value.joints}"
    )


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Embed manual QnBot SDK updates in an application loop"
    )
    arguments.add_argument("--port", required=True, help="Serial port for the glove")
    arguments.add_argument(
        "--side",
        choices=(Side.LEFT.value, Side.RIGHT.value),
        required=True,
        help="Physical glove side",
    )
    arguments.add_argument("--updates", type=int, default=100)
    arguments.add_argument("--package-id", required=True)
    options = arguments.parse_args()
    if options.updates < 1:
        arguments.error("--updates must be at least 1")

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
    output = device.output(name="openxr_hand")
    try:
        glove.start()
        for _ in range(options.updates):
            update = glove.update()
            update.sleep()
            sample = output.latest()
            if sample is not None:
                print_output(sample)
    finally:
        glove.close()


if __name__ == "__main__":
    main()
