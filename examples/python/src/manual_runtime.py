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
    arguments.add_argument("--target-name", default=DEFAULT_TARGET_NAME)
    arguments.add_argument("--package-id", default=DEFAULT_PACKAGE_ID)
    options = arguments.parse_args()
    if options.updates < 1:
        arguments.error("--updates must be at least 1")

    sdk = create_sdk(
        options.port,
        Side(options.side),
        options.target_name,
        options.package_id,
    )
    try:
        glove = sdk.glove()
        glove.connect()
        output = glove.device().output(name=options.target_name)
        glove.start()

        for _ in range(options.updates):
            update = glove.update()
            print(
                f"update tick={update.tick} tasks={update.ran_task_count} "
                f"has_next={update.has_next_task}"
            )
            if update.has_next_task:
                print(f"wait={update.sleep().value}")
            sample = output.latest()
            if sample is not None:
                print_output(sample)

        health = glove.health()
        print(
            f"health ok={health.ok} warnings={health.warning_count} "
            f"errors={health.error_count}"
        )
    finally:
        glove.close()


if __name__ == "__main__":
    main()
