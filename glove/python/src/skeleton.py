from __future__ import annotations

import argparse

from qnbot_sdk import Sample, Sdk, SerialConnection, Side
from qnbot_sdk.glove import GloveConfig, HandJointCommand, HandSkeletonPose


def print_joint_angles(sample: Sample[HandJointCommand]) -> None:
    print(
        f"skeleton sequence={sample.sequence} "
        f"target={sample.value.target} joints={sample.value.joints}"
    )


def print_pose(sample: Sample[HandSkeletonPose]) -> None:
    wrist = sample.value.positions_local[0]
    index_tip = sample.value.positions_local[9]
    print(
        f"pose sequence={sample.sequence} frame={sample.value.coordinate_frame} "
        f"wrist={wrist} index_tip={index_tip}"
    )


def main() -> None:
    arguments = argparse.ArgumentParser(description="Read SDK-owned skeleton output")
    arguments.add_argument("--port", required=True, help="Serial port for the glove")
    arguments.add_argument(
        "--side",
        choices=(Side.LEFT.value, Side.RIGHT.value),
        required=True,
        help="Physical glove side",
    )
    options = arguments.parse_args()
    sdk = Sdk(
        devices=(
            GloveConfig(
                side=Side(options.side),
                connection=SerialConnection(port=options.port),
            ),
        )
    )
    glove = sdk.glove()
    skeleton = glove.device().skeleton()
    skeleton.joint_angles().subscribe(print_joint_angles)
    skeleton.pose().subscribe(print_pose)

    glove.start()
    print("running; press Ctrl+C to stop", flush=True)
    glove.run_forever()


if __name__ == "__main__":
    main()
