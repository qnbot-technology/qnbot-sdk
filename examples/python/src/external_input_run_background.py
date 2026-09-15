from __future__ import annotations

import argparse
import time

from qnbot_sdk import (
    AlgorithmsConfig,
    CalibrationConfig,
    CalibrationInteractionMode,
    DeviceSelector,
    Sample,
    Sdk,
    Side,
    TargetAlgorithm,
    TargetConfig,
)
from qnbot_sdk.glove import (
    CalibrationJobState,
    CaptureSessionState,
    CaptureStageState,
    ExternalConnection,
    ExternalGloveFrame,
    GloveConfig,
    GloveNodePose,
    GlovePose,
    HandJointCommand,
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
        description="Push external frames while the SDK runs in the background"
    )
    arguments.add_argument("--package-id", required=True)
    options = arguments.parse_args()

    sdk = create_sdk(options.package_id)
    glove = sdk.glove()
    device = glove.device()
    pose = device.pose()
    output = device.output(name="hand")
    capture_progress = device.capture_progress()
    calibration_progress = device.calibration_progress(name="hand")
    control = device.capture_control()
    confirmed_request_ids: set[str] = set()

    def print_pose(sample: Sample[GlovePose]) -> None:
        print(f"pose callback sequence={sample.sequence}")

    def print_output(sample: Sample[HandJointCommand]) -> None:
        print(
            f"retargeting output callback sequence={sample.sequence} "
            f"target={sample.value.target} joints={sample.value.joints}"
        )

    def advance_capture() -> None:
        sample = capture_progress.latest()
        if sample is None:
            return
        value = sample.value
        if value.session_state is CaptureSessionState.FAILED:
            message = (
                value.failure.message if value.failure is not None else "unknown error"
            )
            raise RuntimeError(f"external input capture failed: {message}")
        stage = value.stage
        if (
            stage.state is not CaptureStageState.AWAITING_CONFIRMATION
            or stage.request_id is None
            or stage.request_id in confirmed_request_ids
        ):
            return
        answer = input(f"{stage.prompt or 'Continue capture'} [Y/n]: ").strip().lower()
        if answer not in ("", "y", "yes"):
            raise RuntimeError("external input capture was not confirmed")
        control.confirm(stage.request_id)
        confirmed_request_ids.add(stage.request_id)

    pose_subscription = pose.subscribe(print_pose)
    output_subscription = output.subscribe(print_output)
    device.start()
    glove.run_background()

    deadline = time.monotonic() + 60.0
    step = 1
    while output.latest() is None and time.monotonic() < deadline:
        current = device.push_frame(frame(step))
        if step == 1:
            print(f"pushed pose sequence={current.meta.sequence}")
        advance_capture()
        calibration = calibration_progress.latest()
        if (
            calibration is not None
            and calibration.value.job.state is CalibrationJobState.FAILED
        ):
            failure = calibration.value.job.failure
            raise RuntimeError(
                "external input calibration failed: "
                + (failure.message if failure is not None else "unknown error")
            )
        step += 1
        time.sleep(0.01)

    latest_pose = pose.latest()
    latest_output = output.latest()
    if latest_pose is None or latest_output is None:
        raise RuntimeError("external input did not publish pose and retargeting output")
    print(f"latest pose sequence={latest_pose.sequence}")
    print(f"latest retargeting output sequence={latest_output.sequence}")

    glove.request_stop()
    glove.join()
    glove.close()
    _ = pose_subscription, output_subscription
    print("external background input complete")


if __name__ == "__main__":
    main()
