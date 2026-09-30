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
        description="Run the Glove domain on the current thread"
    )
    arguments.add_argument("--port", required=True, help="Serial port for the glove")
    arguments.add_argument(
        "--side",
        choices=(Side.LEFT.value, Side.RIGHT.value),
        required=True,
        help="Physical glove side",
    )
    arguments.add_argument("--package-id", required=True)
    options = arguments.parse_args()

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
        print("running; press Ctrl+C to stop")
        glove.run_forever()
    finally:
        glove.close()


if __name__ == "__main__":
    main()
