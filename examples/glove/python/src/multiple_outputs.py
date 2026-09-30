from __future__ import annotations

import argparse
from collections.abc import Callable

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


def print_output(name: str) -> Callable[[Sample[HandJointCommand]], None]:
    def show(sample: Sample[HandJointCommand]) -> None:
        print(
            f"callback {name} sequence={sample.sequence} "
            f"target={sample.value.target} joints={sample.value.joints}"
        )

    return show


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Read multiple hand outputs driven by one glove"
    )
    arguments.add_argument("--port", required=True, help="Serial port for the glove")
    arguments.add_argument(
        "--side",
        choices=(Side.LEFT.value, Side.RIGHT.value),
        required=True,
        help="Physical glove side",
    )
    arguments.add_argument(
        "--package-id",
        required=True,
        help="Installed algorithm package ID used by both output instances",
    )
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
                name="primary",
                side=side,
                source=source,
                algorithms=(TargetAlgorithm(id=options.package_id),),
            ),
            TargetConfig(
                name="backup",
                side=side,
                source=source,
                algorithms=(TargetAlgorithm(id=options.package_id),),
            ),
        ),
    )
    glove = sdk.glove()
    device = glove.device()

    try:
        device.output(name="primary").subscribe(print_output("primary"))
        device.output(name="backup").subscribe(print_output("backup"))
        glove.start()
        print("running; press Ctrl+C to stop")
        glove.run_forever()
    finally:
        glove.close()


if __name__ == "__main__":
    main()
