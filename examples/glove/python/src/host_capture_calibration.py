from __future__ import annotations

import argparse
import time

from qnbot_sdk import (
    DeviceSelector,
    Sdk,
    Side,
    TargetAlgorithm,
    TargetConfig,
)
from qnbot_sdk.glove import (
    CalibrationJobState,
    CalibrationSessionState,
    CaptureSession,
    CaptureSessionState,
    CaptureStageState,
    Glove,
    GloveConfig,
    GloveDevice,
)

OPERATOR_ID = "default"
LEFT_SOURCE = "primary-glove-left"
RIGHT_SOURCE = "primary-glove-right"
TARGET_NAME = "openxr_hand"
BUILTIN_SKELETON_PACKAGE_ID = "qnbot_hand.dynamic_openxr_hand"
WAIT_SECONDS = 300.0


def configured_targets(package_ids: tuple[str, ...]) -> tuple[tuple[str, str], ...]:
    ordinary_count = sum(
        package_id != BUILTIN_SKELETON_PACKAGE_ID for package_id in package_ids
    )
    ordinary_index = 0
    targets: list[tuple[str, str]] = []
    for package_id in package_ids:
        if package_id == BUILTIN_SKELETON_PACKAGE_ID:
            targets.append((package_id, "skeleton"))
            continue
        ordinary_index += 1
        name = TARGET_NAME if ordinary_count == 1 else f"{TARGET_NAME}_{ordinary_index}"
        targets.append((package_id, name))
    return tuple(targets)


def create_sdk(package_ids: tuple[str, ...]) -> Sdk:
    configured = configured_targets(package_ids)
    return Sdk(
        devices=(
            GloveConfig(name=LEFT_SOURCE, side=Side.LEFT),
            GloveConfig(name=RIGHT_SOURCE, side=Side.RIGHT),
        ),
        targets=tuple(
            TargetConfig(
                name=target_name,
                side=side,
                source=DeviceSelector(type="glove", name=source),
                algorithms=(TargetAlgorithm(id=package_id),),
            )
            for package_id, target_name in configured
            if package_id != BUILTIN_SKELETON_PACKAGE_ID
            for side, source in (
                (Side.LEFT, LEFT_SOURCE),
                (Side.RIGHT, RIGHT_SOURCE),
            )
        ),
    )


def select_target(package_ids: tuple[str, ...]) -> str:
    targets = configured_targets(package_ids)
    if len(targets) == 1:
        return targets[0][1]
    print("Select one configured target for Calibration:")
    for index, (package_id, target_name) in enumerate(targets, start=1):
        print(f"  {index}: {target_name} ({package_id})")
    while True:
        answer = input(f"target [1-{len(targets)}]: ").strip()
        if answer.isdigit() and 1 <= int(answer) <= len(targets):
            return targets[int(answer) - 1][1]
        print("enter one of the listed target numbers")


def capture_side(
    device: GloveDevice,
    glove: Glove,
    package_ids: tuple[str, ...],
    force: bool,
    start_runtime: bool,
) -> CaptureSession:
    readiness = device.capture_status(
        package_ids=package_ids,
        operator_id=OPERATOR_ID,
    )
    print(f"{device.source_id}: capture readiness={readiness.state.value}")
    for stage in readiness.stages:
        print(f"  {stage.stage_id}: {stage.state.value}")

    session = device.start_capture(
        package_ids=package_ids,
        operator_id=OPERATOR_ID,
        force=force,
    )
    if start_runtime:
        glove.start()
    deadline = time.monotonic() + WAIT_SECONDS
    while time.monotonic() < deadline:
        snapshot = session.snapshot()
        if snapshot.state is CaptureSessionState.COMPLETED:
            if session.result() is None:
                raise RuntimeError("completed capture session has no result")
            print(f"{device.source_id}: capture completed")
            return session
        if snapshot.state in (
            CaptureSessionState.FAILED,
            CaptureSessionState.CANCELLED,
        ):
            raise RuntimeError(
                snapshot.failure.message
                if snapshot.failure is not None
                else f"{device.source_id} capture stopped"
            )

        current = next(
            (
                stage
                for stage in snapshot.stage_progress
                if stage.stage_id == snapshot.current_stage
            ),
            None,
        )
        if (
            current is not None
            and current.state is CaptureStageState.AWAITING_CONFIRMATION
        ):
            if snapshot.request_id is None:
                session.cancel()
                raise RuntimeError("capture confirmation is missing its request token")
            prompt = next(
                (
                    stage.prompt
                    for stage in snapshot.plan.stages
                    if stage.stage_id == snapshot.current_stage
                ),
                snapshot.current_stage or "Continue capture",
            )
            answer = input(f"{device.source_id} {prompt} [Y/n]: ").strip().lower()
            if answer in ("", "confirm", "y", "yes"):
                session.confirm(snapshot.request_id)
            else:
                session.cancel()
                raise RuntimeError("capture cancelled by operator")
            continue

        update = glove.update()
        update.sleep()

    session.cancel()
    raise TimeoutError(f"{device.source_id} capture timed out")


def calibrate(
    device: GloveDevice,
    glove: Glove,
    capture_session: CaptureSession,
    targets: tuple[str, ...],
) -> None:
    calibration = device.start_calibration(
        completed_capture_session=capture_session,
        targets=targets,
    )
    deadline = time.monotonic() + WAIT_SECONDS
    while time.monotonic() < deadline:
        snapshot = calibration.snapshot()
        if snapshot.state is CalibrationSessionState.COMPLETED:
            results = calibration.result()
            if results is None:
                calibration.close()
                raise RuntimeError("completed calibration session has no result")
            for result in results:
                print(f"saved calibration record for {result.target_id}")
            calibration.close()
            return
        failed = next(
            (job for job in snapshot.jobs if job.state is CalibrationJobState.FAILED),
            None,
        )
        if failed is not None:
            calibration.close()
            raise RuntimeError(
                failed.failure.message
                if failed.failure is not None
                else "calibration failed"
            )
        if snapshot.state is CalibrationSessionState.CANCELLED:
            calibration.close()
            raise RuntimeError("calibration cancelled")
        update = glove.update()
        update.sleep()

    calibration.close()
    raise TimeoutError("calibration timed out")


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Capture both hands, then calibrate one configured target"
    )
    arguments.add_argument(
        "--package-id",
        action="append",
        dest="package_ids",
        required=True,
        metavar="PACKAGE_ID",
    )
    arguments.add_argument("--force", action="store_true")
    options = arguments.parse_args()
    package_ids = tuple(dict.fromkeys(options.package_ids))

    sdk = create_sdk(package_ids)
    glove = sdk.glove()
    left_device = glove.device(LEFT_SOURCE)
    right_device = glove.device(RIGHT_SOURCE)
    if BUILTIN_SKELETON_PACKAGE_ID in package_ids:
        left_device.skeleton()
        right_device.skeleton()

    left_session = capture_side(
        left_device, glove, package_ids, options.force, start_runtime=True
    )
    right_session = capture_side(
        right_device, glove, package_ids, options.force, start_runtime=False
    )
    targets = (select_target(left_session.snapshot().plan.package_ids),)
    calibrate(left_device, glove, left_session, targets)
    calibrate(right_device, glove, right_session, targets)

    right_session.close()
    left_session.close()
    glove.close()
    print("Calibration results are saved and applied to retargeting by the SDK.")


if __name__ == "__main__":
    main()
