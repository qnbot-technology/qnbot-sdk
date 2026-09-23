from __future__ import annotations

import argparse
import asyncio
import time

from qnbot_sdk import (
    AlgorithmsConfig,
    CaptureConfig,
    CaptureControl,
    CaptureInteractionMode,
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
        algorithms=AlgorithmsConfig(
            capture=CaptureConfig(
                interaction=CaptureInteractionMode.EXTERNAL,
            ),
        ),
    )


def print_output(origin: str, sample: Sample[HandJointCommand]) -> None:
    print(
        f"{origin} retargeting sequence={sample.sequence} "
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


def wait_for_output(
    output: ReadChannel[HandJointCommand],
    capture_progress: ReadChannel[CaptureProgress],
    capture_control: CaptureControl,
    calibration_progress: ReadChannel[CalibrationProgress],
) -> None:
    handled_request_ids: set[str] = set()
    deadline = time.monotonic() + 300.0
    while output.latest() is None and time.monotonic() < deadline:
        handle_capture_prompt(
            capture_progress,
            capture_control,
            calibration_progress,
            handled_request_ids,
        )
        time.sleep(0.05)
    if output.latest() is None:
        raise RuntimeError("retargeting output was not ready before the timeout")


async def run(options: argparse.Namespace) -> None:
    sdk = create_sdk(
        options.port,
        Side(options.side),
        options.target_name,
        options.package_id,
    )
    glove = sdk.glove()
    device = glove.device()
    output = device.output(name=options.target_name)
    capture_progress = device.capture_progress()
    calibration_progress = device.calibration_progress(name=options.target_name)
    capture_control = device.capture_control()
    glove.start()
    glove.run_background()

    await asyncio.to_thread(
        wait_for_output,
        output,
        capture_progress,
        capture_control,
        calibration_progress,
    )

    print_output("next", await output.next())
    received = 0
    async for sample in output:
        print_output("stream", sample)
        received += 1
        if received >= options.samples:
            break

    glove.request_stop()
    await asyncio.to_thread(glove.join)
    await asyncio.to_thread(glove.close)
    print("async stopped")


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Consume retargeted hand output with Python asyncio"
    )
    arguments.add_argument("--port", required=True, help="Serial port for the glove")
    arguments.add_argument(
        "--side",
        choices=(Side.LEFT.value, Side.RIGHT.value),
        required=True,
        help="Physical glove side",
    )
    arguments.add_argument("--samples", type=int, default=10)
    arguments.add_argument("--target-name", default=DEFAULT_TARGET_NAME)
    arguments.add_argument("--package-id", required=True)
    options = arguments.parse_args()
    if options.samples < 1:
        arguments.error("--samples must be at least 1")
    asyncio.run(run(options))


if __name__ == "__main__":
    main()
