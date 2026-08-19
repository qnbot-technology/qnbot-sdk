from __future__ import annotations

import argparse

from qnbot_sdk import (
    AmbiguousSelectionError,
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


def create_sdk(
    left_port: str,
    right_port: str,
    target_name: str,
    algorithm_id: str,
) -> Sdk:
    return Sdk(
        devices=(
            GloveConfig(
                name="left",
                side=Side.LEFT,
                connection=SerialConnection(port=left_port),
            ),
            GloveConfig(
                name="right",
                side=Side.RIGHT,
                connection=SerialConnection(port=right_port),
            ),
        ),
        targets=(
            TargetConfig(
                type=TargetType.HAND,
                name=target_name,
                side=Side.LEFT,
                source=DeviceSelector(type="glove", name="left"),
                algorithms=(TargetAlgorithm(id=algorithm_id),),
            ),
            TargetConfig(
                type=TargetType.HAND,
                name=target_name,
                side=Side.RIGHT,
                source=DeviceSelector(type="glove", name="right"),
                algorithms=(TargetAlgorithm(id=algorithm_id),),
            ),
        ),
    )


def print_output(device_name: str, sample: Sample[HandJointCommand]) -> None:
    print(
        f"{device_name} retargeting sequence={sample.sequence} "
        f"target={sample.value.target} joints={sample.value.joints}"
    )


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Control one physical glove in a multi-glove domain"
    )
    arguments.add_argument("--left-port", required=True)
    arguments.add_argument("--right-port", required=True)
    arguments.add_argument("--updates", type=int, default=10)
    arguments.add_argument("--target-name", default=DEFAULT_TARGET_NAME)
    arguments.add_argument("--algorithm-id", default=DEFAULT_ALGORITHM_ID)
    options = arguments.parse_args()
    if options.updates < 1:
        arguments.error("--updates must be at least 1")

    sdk = create_sdk(
        options.left_port,
        options.right_port,
        options.target_name,
        options.algorithm_id,
    )
    glove = sdk.glove()
    try:
        glove.connect()
        try:
            glove.device()
        except AmbiguousSelectionError:
            print("selection requires name or side")

        left = glove.device(name="left")
        right = glove.device(side=Side.RIGHT)
        left_output = left.output(name=options.target_name)
        right_output = right.output(name=options.target_name)
        left.start()
        right.start()

        for _ in range(options.updates):
            update = glove.update()
            if update.has_next_task:
                update.sleep()
            left_sample = left_output.latest()
            if left_sample is not None:
                print_output("left", left_sample)
            right_sample = right_output.latest()
            if right_sample is not None:
                print_output("right", right_sample)

        right.stop()
        right.disconnect()
        print("right device disconnected")

        health = glove.health()
        print(
            f"health ok={health.ok} warnings={health.warning_count} "
            f"errors={health.error_count}"
        )
    finally:
        glove.close()


if __name__ == "__main__":
    main()
