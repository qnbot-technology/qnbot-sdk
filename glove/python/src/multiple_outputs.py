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


def create_sdk(
    port: str,
    side: Side,
    package_id: str,
) -> Sdk:
    source = DeviceSelector(type="glove", name="primary")
    return Sdk(
        devices=(
            GloveConfig(
                name="primary",
                side=side,
                connection=SerialConnection(port=port),
            ),
        ),
        targets=(
            TargetConfig(
                name="primary",
                side=side,
                source=source,
                algorithms=(TargetAlgorithm(id=package_id),),
            ),
            TargetConfig(
                name="backup",
                side=side,
                source=source,
                algorithms=(TargetAlgorithm(id=package_id),),
            ),
        ),
    )


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

    sdk = create_sdk(
        options.port,
        Side(options.side),
        options.package_id,
    )
    glove = sdk.glove()
    device = glove.device()
    primary = device.output(name="primary")
    backup = device.output(name="backup")
    primary.subscribe(print_output("primary"))
    backup.subscribe(print_output("backup"))

    glove.start()
    print("running; press Ctrl+C to stop", flush=True)
    glove.run_forever()


if __name__ == "__main__":
    main()
