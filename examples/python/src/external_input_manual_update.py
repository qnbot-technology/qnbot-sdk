from __future__ import annotations

import argparse
import time

from qnbot_sdk import (
    AlgorithmsConfig,
    CalibrationConfig,
    CalibrationInteractionMode,
    CalibrationProgressState,
    DeviceSelector,
    Sdk,
    Side,
    TargetAlgorithm,
    TargetConfig,
    TargetType,
)
from qnbot_sdk.glove import (
    ExternalConnection,
    ExternalGloveFrame,
    GloveConfig,
    GloveNodePose,
)


def node(x: float, y: float, z: float) -> GloveNodePose:
    return GloveNodePose(
        position=(x, y, z),
        quaternion_xyzw=(0.0, 0.0, 0.0, 1.0),
    )


def frame(step: int) -> ExternalGloveFrame:
    offset = step * 0.001
    return ExternalGloveFrame(
        thumb=node(0.01 + offset, 0.02, 0.03),
        index=node(0.02 + offset, 0.03, 0.04),
        middle=node(0.03 + offset, 0.04, 0.05),
        ring=node(0.04 + offset, 0.05, 0.06),
        pinky=node(0.05 + offset, 0.06, 0.07),
        timestamp_sec=step * 0.01,
    )


def create_sdk(package_id: str) -> Sdk:
    return Sdk(
        devices=(GloveConfig(side=Side.RIGHT, connection=ExternalConnection()),),
        targets=(
            TargetConfig(
                type=TargetType.HAND,
                name="hand",
                side=Side.RIGHT,
                source=DeviceSelector(type="glove", side=Side.RIGHT),
                algorithms=(TargetAlgorithm(id=package_id),),
            ),
        ),
        algorithms=AlgorithmsConfig(
            calibration=CalibrationConfig(
                interaction=CalibrationInteractionMode.EXTERNAL,
            ),
        ),
    )


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Push external frames and update the SDK from the application loop"
    )
    arguments.add_argument("--package-id", required=True)
    options = arguments.parse_args()

    sdk = create_sdk(options.package_id)
    glove = sdk.glove()
    glove.connect()
    device = glove.device()
    pose = device.pose()
    output = device.output(name="hand")
    progress = device.calibration_progress(name="hand")
    control = device.calibration_control(name="hand")
    confirmed_request_ids: set[str] = set()

    def confirm_calibration() -> None:
        sample = progress.latest()
        if sample is None:
            return
        value = sample.value
        if value.state is CalibrationProgressState.FAILED:
            message = (
                value.failure.message if value.failure is not None else "unknown error"
            )
            raise RuntimeError(f"external input calibration failed: {message}")
        if (
            value.state is not CalibrationProgressState.AWAITING_CONFIRMATION
            or value.request_id is None
            or value.request_id in confirmed_request_ids
        ):
            return
        answer = (
            input(f"{value.prompt or 'Continue calibration'} [Y/n]: ").strip().lower()
        )
        if answer not in ("", "y", "yes"):
            raise RuntimeError("external input calibration was not confirmed")
        control.confirm(value.request_id)
        confirmed_request_ids.add(value.request_id)

    try:
        device.start()
        deadline = time.monotonic() + 60.0
        step = 1
        while output.latest() is None and time.monotonic() < deadline:
            current = device.push_frame(frame(step))
            if step == 1:
                print(f"pushed pose sequence={current.meta.sequence}")
            update = glove.update()
            confirm_calibration()
            step += 1
            if update.has_next_task:
                update.sleep()
            else:
                time.sleep(0.01)

        latest_pose = pose.latest()
        latest_output = output.latest()
        if latest_pose is None or latest_output is None:
            raise RuntimeError("external input did not publish retargeting output")
        print(f"latest pose sequence={latest_pose.sequence}")
        print(
            f"latest retargeting output sequence={latest_output.sequence} "
            f"target={latest_output.value.target} "
            f"joints={latest_output.value.joints}"
        )
        print("external manual input complete")
    finally:
        glove.close()


if __name__ == "__main__":
    main()
