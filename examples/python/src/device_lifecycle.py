from __future__ import annotations

import argparse
import time

from qnbot_sdk import (
    AlgorithmsConfig,
    CalibrationConfig,
    CalibrationInteractionMode,
    CaptureControl,
    DeviceSelector,
    ReadChannel,
    Sample,
    Sdk,
    SerialConnection,
    Side,
    TargetAlgorithm,
    TargetConfig,
)
from qnbot_sdk.glove import (
    CalibrationJobState,
    CalibrationProgress,
    CaptureProgress,
    CaptureSessionState,
    CaptureStageState,
    GloveConfig,
    HandJointCommand,
)

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
        algorithms=AlgorithmsConfig(
            calibration=CalibrationConfig(
                interaction=CalibrationInteractionMode.EXTERNAL,
            ),
        ),
    )


def print_output(device_name: str, sample: Sample[HandJointCommand]) -> None:
    print(
        f"{device_name} retargeting sequence={sample.sequence} "
        f"target={sample.value.target} joints={sample.value.joints}"
    )


def handle_capture_prompt(
    capture_progress: ReadChannel[CaptureProgress],
    capture_control: CaptureControl,
    calibration_progress: ReadChannel[CalibrationProgress],
    handled_request_ids: set[str],
) -> None:
    capture = capture_progress.latest()
    if capture is not None:
        value = capture.value
        if value.session_state is CaptureSessionState.FAILED:
            message = value.failure.message if value.failure else "unknown error"
            raise RuntimeError(f"capture failed: {message}")
        stage = value.stage
        if (
            stage.state is CaptureStageState.AWAITING_CONFIRMATION
            and stage.request_id is not None
            and stage.request_id not in handled_request_ids
        ):
            answer = (
                input(f"{stage.prompt or 'Continue capture'} [Y/n]: ").strip().lower()
            )
            if answer in ("", "y", "yes"):
                capture_control.confirm(stage.request_id)
            else:
                capture_control.cancel(stage.request_id)
                raise RuntimeError("capture was cancelled")
            handled_request_ids.add(stage.request_id)
    calibration = calibration_progress.latest()
    if (
        calibration is not None
        and calibration.value.job.state is CalibrationJobState.FAILED
    ):
        failure = calibration.value.job.failure
        message = failure.message if failure else "unknown error"
        raise RuntimeError(f"calibration failed: {message}")


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
    left_capture_progress = left.capture_progress()
    right_capture_progress = right.capture_progress()
    left_calibration_progress = left.calibration_progress(name=options.target_name)
    right_calibration_progress = right.calibration_progress(name=options.target_name)
    left_capture_control = left.capture_control()
    right_capture_control = right.capture_control()
    left.start()
    right.start()

    def wait_for_output(
        output: ReadChannel[HandJointCommand],
        capture_progress: ReadChannel[CaptureProgress],
        capture_control: CaptureControl,
        calibration_progress: ReadChannel[CalibrationProgress],
    ) -> None:
        handled_request_ids: set[str] = set()
        deadline = time.monotonic() + 300.0
        while output.latest() is None and time.monotonic() < deadline:
            update = glove.update()
            handle_capture_prompt(
                capture_progress,
                capture_control,
                calibration_progress,
                handled_request_ids,
            )
            if update.has_next_task:
                update.sleep()
        if output.latest() is None:
            raise RuntimeError("retargeting output was not ready before the timeout")

    wait_for_output(
        left_output,
        left_capture_progress,
        left_capture_control,
        left_calibration_progress,
    )
    wait_for_output(
        right_output,
        right_capture_progress,
        right_capture_control,
        right_calibration_progress,
    )

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
