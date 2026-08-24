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

DEFAULT_PACKAGE_ID = "qnbot-dexhand"
DEFAULT_TARGET_NAME = "openxr_hand"


def create_sdk(
    left_port: str,
    right_port: str,
    target_name: str,
    package_id: str,
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
                algorithms=(TargetAlgorithm(id=package_id),),
            ),
            TargetConfig(
                type=TargetType.HAND,
                name=target_name,
                side=Side.RIGHT,
                source=DeviceSelector(type="glove", name="right"),
                algorithms=(TargetAlgorithm(id=package_id),),
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
    arguments.add_argument("--package-id", default=DEFAULT_PACKAGE_ID)
    options = arguments.parse_args()
    if options.updates < 1:
        arguments.error("--updates must be at least 1")

    sdk = create_sdk(
        options.left_port,
        options.right_port,
        options.target_name,
        options.package_id,
    )
    glove = sdk.glove()
    try:
        glove.connect()
        try:
            glove.device()
        except AmbiguousSelectionError:
            print("selection requires name or side")
        else:
            raise RuntimeError("unqualified device selection was not ambiguous")

        left = glove.device(name="left")
        right = glove.device(side=Side.RIGHT)
        left_output = left.output(name=options.target_name)
        right_output = right.output(name=options.target_name)
        left.start()
        right.start()

        left_sequence: int | None = None
        right_sequence: int | None = None
        for _ in range(options.updates):
            update = glove.update()
            if update.has_next_task:
                update.sleep()
            left_sample = left_output.latest()
            if left_sample is not None and left_sample.sequence != left_sequence:
                print_output("left", left_sample)
                left_sequence = left_sample.sequence
            right_sample = right_output.latest()
            if right_sample is not None and right_sample.sequence != right_sequence:
                print_output("right", right_sample)
                right_sequence = right_sample.sequence
            if left_sequence is not None and right_sequence is not None:
                break

        if left_sequence is None or right_sequence is None:
            raise RuntimeError("both gloves must produce output before disconnect")

        right.stop()
        right.disconnect()
        print("right device disconnected")

        for _ in range(options.updates):
            update = glove.update()
            if update.has_next_task:
                update.sleep()
            left_sample = left_output.latest()
            if left_sample is not None and left_sample.sequence != left_sequence:
                print_output("left after right disconnect", left_sample)
                break
        else:
            raise RuntimeError("left glove stopped producing after right disconnect")

        health = glove.health()
        print(
            f"health ok={health.ok} warnings={health.warning_count} "
            f"errors={health.error_count}"
        )
    finally:
        glove.close()


if __name__ == "__main__":
    main()
