from __future__ import annotations

import argparse
import time

from qnbot_sdk import (
    AlgorithmsConfig,
    CaptureConfig,
    CaptureControl,
    CaptureInteractionMode,
    DeviceSelector,
    ReadChannel,
    Sdk,
    Side,
    SleepResult,
    TargetAlgorithm,
    TargetConfig,
)
from qnbot_sdk.glove import (
    CalibrationJobState,
    CalibrationProgress,
    CaptureProgress,
    CaptureSessionState,
    CaptureStageState,
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
        timestamp_ms=step * 10.0,
    )


def check_preparation(
    capture_progress: ReadChannel[CaptureProgress],
    calibration_progress: ReadChannel[CalibrationProgress],
) -> None:
    capture = capture_progress.latest()
    if capture is not None:
        value = capture.value
        if value.session_state is CaptureSessionState.FAILED:
            message = value.failure.message if value.failure else "unknown error"
            raise RuntimeError(f"external input capture failed: {message}")
        if value.session_state is CaptureSessionState.CANCELLED:
            raise RuntimeError("external input capture was cancelled")
    calibration = calibration_progress.latest()
    if (
        calibration is not None
        and calibration.value.job.state is CalibrationJobState.FAILED
    ):
        failure = calibration.value.job.failure
        message = failure.message if failure else "unknown error"
        raise RuntimeError(f"external input calibration failed: {message}")


def handle_capture_prompt(
    capture_progress: ReadChannel[CaptureProgress],
    capture_control: CaptureControl,
    handled_request_ids: set[str],
) -> None:
    capture = capture_progress.latest()
    if capture is not None:
        stage = capture.value.stage
        if (
            stage.state is CaptureStageState.AWAITING_CONFIRMATION
            and stage.request_id is not None
            and stage.request_id not in handled_request_ids
        ):
            prompt = stage.prompt_en or stage.prompt or "Continue capture"
            answer = input(f"{prompt} [Y/n]: ").strip().lower()
            if answer in ("", "y", "yes"):
                capture_control.confirm(stage.request_id)
            else:
                capture_control.cancel(stage.request_id)
                raise RuntimeError("external input capture was cancelled")
            handled_request_ids.add(stage.request_id)


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Push external frames and update the SDK from the application loop"
    )
    arguments.add_argument("--package-id", required=True)
    arguments.add_argument(
        "--capture-interaction",
        choices=("auto_terminal", "external"),
        default="auto_terminal",
        help="SDK 终端引导（默认）；external 由应用显示提示并确认。标定期间持续推帧。",
    )
    options = arguments.parse_args()

    sdk = Sdk(
        devices=(
            GloveConfig(
                name="external",
                side=Side.RIGHT,
                connection=ExternalConnection(),
            ),
        ),
        targets=(
            TargetConfig(
                name="hand",
                side=Side.RIGHT,
                source=DeviceSelector(type="glove", side=Side.RIGHT),
                algorithms=(TargetAlgorithm(id=options.package_id),),
            ),
        ),
        algorithms=AlgorithmsConfig(
            capture=CaptureConfig(
                interaction=CaptureInteractionMode(options.capture_interaction),
            ),
        ),
    )
    glove = sdk.glove()
    device = glove.device()
    pose = device.pose()
    output = device.output(name="hand")
    capture_progress = device.capture_progress()
    calibration_progress = device.calibration_progress(name="hand")
    control = device.capture_control()
    confirmed_request_ids: set[str] = set()

    try:
        device.start()
        deadline = time.monotonic() + 60.0
        step = 1
        while output.latest() is None and time.monotonic() < deadline:
            check_preparation(capture_progress, calibration_progress)
            device.push_frame(frame(step))
            update = glove.update()
            if options.capture_interaction == "external":
                handle_capture_prompt(
                    capture_progress,
                    control,
                    confirmed_request_ids,
                )
            step += 1
            if update.sleep() is SleepResult.NO_TASKS:
                time.sleep(0.01)
        latest_pose = pose.latest()
        latest_output = output.latest()
        if latest_pose is None or latest_output is None:
            raise RuntimeError("external input did not publish retargeting output")
        print(
            f"latest retargeting output sequence={latest_output.sequence} "
            f"target={latest_output.value.target} "
            f"joints={latest_output.value.joints}"
        )
    finally:
        glove.close()
    print("external manual input complete")


if __name__ == "__main__":
    main()
