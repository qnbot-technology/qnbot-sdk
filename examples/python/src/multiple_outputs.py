from __future__ import annotations

import argparse
from collections.abc import Callable
from threading import Lock

from qnbot_sdk import (
    DeviceSelector,
    Sample,
    Sdk,
    SerialConnection,
    Side,
    TargetAlgorithm,
    TargetConfig,
    TargetType,
)
from qnbot_sdk.glove import GloveConfig, HandJointCommand

_PRINT_LOCK = Lock()


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
                type=TargetType.HAND,
                name="primary",
                side=side,
                source=source,
                algorithms=(TargetAlgorithm(id=package_id),),
            ),
            TargetConfig(
                type=TargetType.HAND,
                name="backup",
                side=side,
                source=source,
                algorithms=(TargetAlgorithm(id=package_id),),
            ),
        ),
    )


def print_output(name: str) -> Callable[[Sample[HandJointCommand]], None]:
    def show(sample: Sample[HandJointCommand]) -> None:
        with _PRINT_LOCK:
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
    try:
        glove.connect()
        device = glove.device()
        routes = (
            (
                "primary",
                device.output(name="primary"),
            ),
            (
                "backup",
                device.output(name="backup"),
            ),
        )
        for name, output in routes:
            output.subscribe(print_output(name))

        glove.start()

        try:
            print("ready; press Ctrl+C to stop", flush=True)
            glove.run_forever()
        except KeyboardInterrupt:
            print("\nstopping")
        print("multiple outputs stopped")
    finally:
        glove.close()


if __name__ == "__main__":
    main()
