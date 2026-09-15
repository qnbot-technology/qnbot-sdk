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
                name=target_name,
                side=Side.LEFT,
                source=DeviceSelector(type="glove", name="left"),
                algorithms=(TargetAlgorithm(id=package_id),),
            ),
            TargetConfig(
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
    arguments.add_argument("--package-id", required=True)
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

    left_before_stop = left_output.latest()
    if left_before_stop is None:
        raise RuntimeError("both gloves must produce output before lifecycle control")

    right.stop()
    print("right device stopped; left remains active")
    right_stopped_baseline = right_output.latest()
    if right_stopped_baseline is None:
        raise RuntimeError("right output was unavailable after stop")
    left_sample = None
    for _ in range(options.updates):
        update = glove.update()
        if update.has_next_task:
            update.sleep()
        right_candidate = right_output.latest()
        if (
            right_candidate is not None
            and right_candidate.sequence > right_stopped_baseline.sequence
        ):
            raise RuntimeError("right output advanced while the device was stopped")
        candidate = left_output.latest()
        if candidate is not None and candidate.sequence > left_before_stop.sequence:
            left_sample = candidate
            break
    if left_sample is None:
        raise RuntimeError("left output did not advance while right was stopped")
    print("left output while right stopped")
    print_output("left", left_sample)

    right_before_restart = right_output.latest()
    if right_before_restart is None:
        raise RuntimeError("right output was unavailable before restart")
    right.start()
    print("right device restarted")
    right_sample = None
    for _ in range(options.updates):
        update = glove.update()
        if update.has_next_task:
            update.sleep()
        candidate = right_output.latest()
        if candidate is not None and candidate.sequence > right_before_restart.sequence:
            right_sample = candidate
            break
    if right_sample is None:
        raise RuntimeError("right output did not advance after restart")
    print("right output after restart")
    print_output("right", right_sample)

    health = glove.health()
    print(
        f"health ok={health.ok} warnings={health.warning_count} "
        f"errors={health.error_count}"
    )
    glove.close()


if __name__ == "__main__":
    main()
