from __future__ import annotations

import argparse
from pathlib import Path
from threading import Lock

from qnbot_sdk import (
    DebugConfig,
    DebugDetail,
    DeviceSelector,
    Sample,
    Sdk,
    SerialConnection,
    Side,
    TargetAlgorithm,
    TargetConfig,
)
from qnbot_sdk.glove import GloveConfig, GlovePose, HandJointCommand

DEFAULT_TARGET_NAME = "openxr_hand"
_PRINT_LOCK = Lock()


def create_sdk(
    port: str,
    side: Side,
    log_dir: Path | None,
    sample_rate: int,
    detail: DebugDetail,
    target_name: str,
    package_id: str,
) -> Sdk:
    source = DeviceSelector(type="glove", name="primary")
    modules = (
        "serial",
        "glove",
        "calibration",
        "retargeting",
        "output",
        "buffer",
    )
    debug = (
        DebugConfig(
            modules=modules,
            detail=detail,
            sample_rate=sample_rate,
        )
        if log_dir is None
        else DebugConfig(
            log_dir=log_dir,
            modules=modules,
            detail=detail,
            sample_rate=sample_rate,
        )
    )
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
        debug=debug,
    )


def print_pose(origin: str, sample: Sample[GlovePose]) -> None:
    with _PRINT_LOCK:
        print(f"{origin} pose sequence={sample.sequence}")


def print_output(sample: Sample[HandJointCommand]) -> None:
    with _PRINT_LOCK:
        print(
            f"retargeting sequence={sample.sequence} "
            f"target={sample.value.target} joints={sample.value.joints}"
        )


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Read glove poses with sampled debug tracing enabled"
    )
    arguments.add_argument("--port", required=True, help="Serial port for the glove")
    arguments.add_argument(
        "--side",
        choices=(Side.LEFT.value, Side.RIGHT.value),
        required=True,
        help="Physical glove side",
    )
    arguments.add_argument(
        "--log-dir",
        type=Path,
        default=None,
        help="Override the platform default directory for debug JSONL files",
    )
    arguments.add_argument("--sample-rate", type=int, default=10)
    arguments.add_argument(
        "--detail",
        choices=tuple(detail.value for detail in DebugDetail),
        default=DebugDetail.SUMMARY.value,
    )
    arguments.add_argument("--target-name", default=DEFAULT_TARGET_NAME)
    arguments.add_argument("--package-id", required=True)
    options = arguments.parse_args()
    if options.sample_rate <= 0:
        arguments.error("--sample-rate must be greater than 0")

    log_dir = options.log_dir
    trace_dir = log_dir if log_dir is not None else DebugConfig().log_dir
    sdk = create_sdk(
        options.port,
        Side(options.side),
        log_dir,
        options.sample_rate,
        DebugDetail(options.detail),
        options.target_name,
        options.package_id,
    )
    glove = sdk.glove()
    try:
        glove.connect()
        device = glove.device()
        pose = device.pose()
        output = device.output(name=options.target_name)
        pose.subscribe(lambda sample: print_pose("callback", sample))
        output.subscribe(print_output)

        glove.start()
        print(f"debug traces: {trace_dir}")

        try:
            print("ready; press Ctrl+C to stop", flush=True)
            glove.run_forever()
        except KeyboardInterrupt:
            print("\nstopping")

        latest = pose.latest()
        if latest is not None:
            print_pose("latest", latest)
        health = glove.health()
        print(
            f"health ok={health.ok} warnings={health.warning_count} "
            f"errors={health.error_count}"
        )
    finally:
        glove.close()


if __name__ == "__main__":
    main()
