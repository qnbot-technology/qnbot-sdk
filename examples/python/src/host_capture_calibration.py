from __future__ import annotations

import argparse

from qnbot_sdk import DeviceSelector, Sdk, Side, TargetConfig, TargetType
from qnbot_sdk.glove import (
    CalibrationJobState,
    CaptureControlAction,
    CaptureReadiness,
    CaptureSessionState,
    CaptureSessionSnapshot,
    CaptureSet,
    CaptureStageState,
    Glove,
    GloveConfig,
)

OPERATOR_ID = "default"
LEFT_SOURCE = "primary-glove-left"
RIGHT_SOURCE = "primary-glove-right"
TARGET_NAME = "openxr_hand"


def create_sdk() -> Sdk:
    return Sdk(
        devices=(
            GloveConfig(name=LEFT_SOURCE, side=Side.LEFT),
            GloveConfig(name=RIGHT_SOURCE, side=Side.RIGHT),
        ),
        targets=(
            TargetConfig(
                type=TargetType.HAND,
                name=TARGET_NAME,
                side=Side.LEFT,
                source=DeviceSelector(type="glove", name=LEFT_SOURCE),
                algorithms=(),
            ),
            TargetConfig(
                type=TargetType.HAND,
                name=TARGET_NAME,
                side=Side.RIGHT,
                source=DeviceSelector(type="glove", name=RIGHT_SOURCE),
                algorithms=(),
            ),
        ),
    )


def render_capture_readiness(side: Side, readiness: CaptureReadiness) -> None:
    resolved = ", ".join(
        f"{package.id}@{package.version}"
        for package in readiness.plan.resolved_packages
    )
    print(f"{side.value}: capture readiness={readiness.state.value} plan={resolved}")
    readiness_by_stage = {stage.stage_id: stage for stage in readiness.stages}
    for stage in readiness.plan.stages:
        current = readiness_by_stage[stage.stage_id]
        requirement = "optional" if stage.optional else "required"
        print(
            f"  {stage.stage_id}: {current.state.value}, {requirement}, "
            f"samples={stage.required_sample_count}, prompt={stage.prompt}"
        )


def render_session_snapshot(side: Side, snapshot: CaptureSessionSnapshot) -> None:
    operations = ",".join(action.value for action in snapshot.allowed_operations)
    print(
        f"{side.value}: session={snapshot.state.value} "
        f"current={snapshot.current_stage or '-'} "
        f"progress={snapshot.collected_sample_count}/"
        f"{snapshot.required_sample_count} next={snapshot.next_action} "
        f"allowed={operations or 'none'}"
    )
    for stage in snapshot.stage_runs:
        print(
            f"  {stage.stage_id}: state={stage.state.value}, "
            f"origin={stage.origin.value}, "
            f"samples={stage.collected_sample_count}/"
            f"{stage.required_sample_count}"
        )


def render_capture_set(side: Side, capture_set: CaptureSet) -> None:
    snapshot = capture_set.snapshot()
    print(f"{side.value}: completed CaptureSet source={snapshot.source_id}")
    for stage in snapshot.stage_samples:
        print(f"  {stage.stage_id}: saved samples={stage.frame_count}")


def capture_side(
    glove: Glove,
    candidate_package_ids: tuple[str, ...],
    source: str,
    side: Side,
    force: bool,
) -> CaptureSet:
    readiness = glove.capture_status(
        candidate_ids=candidate_package_ids,
        operator_id=OPERATOR_ID,
        source=source,
        side=side,
    )
    render_capture_readiness(side, readiness)
    session = glove.start_capture(
        candidate_ids=candidate_package_ids,
        operator_id=OPERATOR_ID,
        source=source,
        side=side,
        force=force,
    )
    try:
        shown_snapshot: CaptureSessionSnapshot | None = None
        while True:
            snapshot = session.snapshot()
            if snapshot != shown_snapshot:
                render_session_snapshot(side, snapshot)
                shown_snapshot = snapshot

            if snapshot.state is CaptureSessionState.COMPLETED:
                result = session.result()
                if result is None:
                    raise RuntimeError("completed capture session has no result")
                render_capture_set(side, result)
                return result
            if snapshot.state in (
                CaptureSessionState.FAILED,
                CaptureSessionState.CANCELLED,
            ):
                raise RuntimeError(snapshot.failure or f"{side.value} capture stopped")
            if snapshot.state is CaptureSessionState.AWAITING_CONFIRMATION:
                if snapshot.request_id is None:
                    raise RuntimeError(
                        "capture confirmation is missing its request token"
                    )
                current_stage = snapshot.current_stage or "current stage"
                prompt = next(
                    (
                        stage.prompt
                        for stage in snapshot.plan.stages
                        if stage.stage_id == snapshot.current_stage
                    ),
                    current_stage,
                )
                allowed = set(snapshot.allowed_operations)
                retryable_stage_ids = tuple(
                    stage.stage_id
                    for stage in snapshot.stage_runs
                    if stage.state
                    in (CaptureStageState.CAPTURED, CaptureStageState.REUSED)
                )
                choices = []
                if CaptureControlAction.CONFIRM in allowed:
                    choices.append("Enter=confirm")
                if CaptureControlAction.SKIP in allowed:
                    choices.append("skip=skip")
                if CaptureControlAction.RETRY in allowed:
                    choices.append(
                        f"retry <stage_id> ({', '.join(retryable_stage_ids)})"
                    )
                if CaptureControlAction.CANCEL in allowed:
                    choices.append("cancel=cancel")
                answer = (
                    input(f"{side.value} {prompt} [{', '.join(choices)}]: ")
                    .strip()
                    .lower()
                )
                action, _, retry_stage_id = answer.partition(" ")
                if answer == "cancel" and CaptureControlAction.CANCEL in allowed:
                    session.cancel()
                elif answer == "skip" and CaptureControlAction.SKIP in allowed:
                    session.skip(snapshot.request_id)
                elif (
                    action == "retry"
                    and CaptureControlAction.RETRY in allowed
                    and retry_stage_id in retryable_stage_ids
                ):
                    session.retry(retry_stage_id)
                elif (
                    answer in ("", "confirm", "y", "yes")
                    and CaptureControlAction.CONFIRM in allowed
                ):
                    session.confirm(snapshot.request_id)
                else:
                    print("action is not allowed in the current session state")
                continue

            update = glove.update()
            if update.has_next_task:
                update.sleep()
    finally:
        session.close()


def choose_package(argument: str | None, candidate_package_ids: tuple[str, ...]) -> str:
    selected = (
        argument
        or input(f"Choose package ({', '.join(candidate_package_ids)}): ").strip()
    )
    if selected not in candidate_package_ids:
        raise ValueError("selected package must be one of the captured candidates")
    return selected


def calibrate(glove: Glove, capture_set: CaptureSet, package_id: str) -> None:
    job = glove.start_calibration(
        capture_set=capture_set,
        package_id=package_id,
        target=TARGET_NAME,
    )
    try:
        shown_state: CalibrationJobState | None = None
        while True:
            snapshot = job.snapshot()
            if snapshot.state is not shown_state:
                print(
                    f"calibration state={snapshot.state.value} "
                    f"package={snapshot.package_id}@{snapshot.package_version}"
                )
                shown_state = snapshot.state
            if snapshot.state is CalibrationJobState.COMPLETED:
                result = job.result()
                if result is None:
                    raise RuntimeError("completed calibration job has no result")
                print(
                    f"saved {result.package_id}@{result.package_version} "
                    f"for {result.target_id}"
                )
                return
            if snapshot.state is CalibrationJobState.FAILED:
                failure = snapshot.failure
                raise RuntimeError(
                    failure.message if failure is not None else "calibration failed"
                )
            update = glove.update()
            if update.has_next_task:
                update.sleep()
    finally:
        job.close()


def main() -> None:
    arguments = argparse.ArgumentParser(
        description=(
            "Inspect reuse, capture both hands for a candidate package set, "
            "then calibrate the selected package"
        )
    )
    arguments.add_argument(
        "--package-id",
        action="append",
        dest="package_ids",
        required=True,
        metavar="PACKAGE_ID",
        help=(
            "candidate package to include in the merged CapturePlan; repeat for "
            "multiple packages"
        ),
    )
    arguments.add_argument("--force", action="store_true")
    options = arguments.parse_args()
    candidate_package_ids = tuple(options.package_ids)

    sdk = create_sdk()
    glove = sdk.glove()
    left_capture_set: CaptureSet | None = None
    right_capture_set: CaptureSet | None = None
    try:
        glove.connect()
        glove.start()

        left_capture_set = capture_side(
            glove,
            candidate_package_ids,
            LEFT_SOURCE,
            side=Side.LEFT,
            force=options.force,
        )
        right_capture_set = capture_side(
            glove,
            candidate_package_ids,
            RIGHT_SOURCE,
            side=Side.RIGHT,
            force=options.force,
        )

        selected_package_id = (
            candidate_package_ids[0]
            if len(candidate_package_ids) == 1
            else choose_package(None, candidate_package_ids)
        )
        calibrate(glove, left_capture_set, selected_package_id)
        calibrate(glove, right_capture_set, selected_package_id)
        print("Calibration results are saved and applied to retargeting by the SDK.")
    finally:
        if right_capture_set is not None:
            right_capture_set.close()
        if left_capture_set is not None:
            left_capture_set.close()
        glove.close()


if __name__ == "__main__":
    main()
