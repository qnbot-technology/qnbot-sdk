from __future__ import annotations

import argparse
import time

from qnbot_sdk import (
    DeviceSelector,
    Health,
    Sample,
    Sdk,
    SerialConnection,
    Side,
    TargetAlgorithm,
    TargetConfig,
)
from qnbot_sdk.glove import GloveConfig, HandJointCommand

DEFAULT_PACKAGE_ID = "qnbot-dexhand"
DEFAULT_TARGET_NAME = "openxr_hand"


def create_sdk(port: str, side: Side, target_name: str, package_id: str) -> Sdk:
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
                name=target_name,
                side=side,
                source=source,
                algorithms=(TargetAlgorithm(id=package_id),),
            ),
        ),
    )


def print_output(sample: Sample[HandJointCommand]) -> None:
    print(
        f"retargeting sequence={sample.sequence} "
        f"target={sample.value.target} joints={sample.value.joints}"
    )


def print_health(label: str, health: Health) -> None:
    devices = ", ".join(
        f"{source_id}(connected={device.connected}, stale={device.stale})"
        for source_id, device in health.devices.items()
    )
    print(
        f"{label} health ok={health.ok} warnings={health.warning_count} "
        f"errors={health.error_count} devices=[{devices}]"
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
    arguments.add_argument("--target-name", default=DEFAULT_TARGET_NAME)
    arguments.add_argument("--package-id", default=DEFAULT_PACKAGE_ID)
    options = arguments.parse_args()
    if options.seconds <= 0:
        arguments.error("--seconds must be greater than 0")

    sdk = create_sdk(
        options.port,
        Side(options.side),
        options.target_name,
        options.package_id,
    )
    glove = sdk.glove()
    try:
        glove.connect()
        output = glove.device().output(name=options.target_name)
        output.subscribe(print_output)
        glove.start()
        glove.run_background()
        try:
            time.sleep(options.seconds)
            print_health("running", glove.health())
        finally:
            glove.request_stop()
            glove.join()
        print("background stopped")
        print_health("stopped lifecycle", glove.health())
    finally:
        glove.close()


if __name__ == "__main__":
    main()
