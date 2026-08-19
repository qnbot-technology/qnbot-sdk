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
    TargetType,
)
from qnbot_sdk.glove import GloveConfig, HandJointCommand

DEFAULT_ALGORITHM_ID = "qnbot.hand.openxr_hand.retargeting"
DEFAULT_TARGET_NAME = "openxr_hand"


def create_sdk(port: str, side: Side, target_name: str, algorithm_id: str) -> Sdk:
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
                name=target_name,
                side=side,
                source=source,
                algorithms=(TargetAlgorithm(id=algorithm_id),),
            ),
        ),
    )


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
    arguments.add_argument("--target-name", default=DEFAULT_TARGET_NAME)
    arguments.add_argument("--algorithm-id", default=DEFAULT_ALGORITHM_ID)
    options = arguments.parse_args()

    sdk = create_sdk(
        options.port,
        Side(options.side),
        options.target_name,
        options.algorithm_id,
    )
    glove = sdk.glove()
    try:
        glove.connect()
        output = glove.device().output(name=options.target_name)
        output.subscribe(print_output)
        glove.start()

        try:
            print("ready; press Ctrl+C to stop", flush=True)
            glove.run_forever()
        except KeyboardInterrupt:
            print("\nstopping")
        print("foreground stopped")

        health = glove.health()
        print(
            f"health ok={health.ok} warnings={health.warning_count} "
            f"errors={health.error_count}"
        )
    finally:
        glove.close()


if __name__ == "__main__":
    main()
