from __future__ import annotations

import argparse
from threading import Lock

from qnbot_sdk import (
    Sample,
    Sdk,
    SerialConnection,
    Side,
    TargetAlgorithm,
    TargetConfig,
    TargetType,
)
from qnbot_sdk.glove import GloveConfig, GlovePose, HandJointCommand


_PRINT_LOCK = Lock()


def print_pose(sample: Sample[GlovePose]) -> None:
    with _PRINT_LOCK:
        print(
            f"pose sequence={sample.sequence} "
            f"fingertips={sample.value.payload.fingertip_local}"
        )


def print_output(sample: Sample[HandJointCommand]) -> None:
    with _PRINT_LOCK:
        print(
            f"retargeting sequence={sample.sequence} "
            f"target={sample.value.target} joints={sample.value.joints}"
        )


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Run the minimal target-output workflow"
    )
    arguments.add_argument("--port", required=True, help="Serial port for the glove")
    arguments.add_argument(
        "--side",
        choices=(Side.LEFT.value, Side.RIGHT.value),
        required=True,
        help="Physical glove side",
    )
    arguments.add_argument("--package-id", required=True)
    options = arguments.parse_args()
    side = Side(options.side)
    sdk = Sdk(
        devices=(
            GloveConfig(
                name="primary",
                side=side,
                connection=SerialConnection(port=options.port),
            ),
        ),
        targets=(
            TargetConfig(
                type=TargetType.HAND,
                name="openxr_hand",
                side=side,
                algorithms=(TargetAlgorithm(options.package_id),),
            ),
        ),
    )
    glove = sdk.glove()
    try:
        glove.connect()
        device = glove.device()
        pose = device.pose()
        output = device.output()
        pose.subscribe(print_pose)
        output.subscribe(print_output)

        glove.start()

        try:
            glove.run_forever()
        except KeyboardInterrupt:
            print("\nstopping")
    finally:
        glove.close()


if __name__ == "__main__":
    main()
