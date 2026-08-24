from __future__ import annotations

from qnbot_sdk import Sample, Sdk
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
    sdk = Sdk(devices=(GloveConfig(),))
    glove = sdk.glove()
    try:
        glove.connect()
        device = glove.device()
        skeleton = device.skeleton()
        angle_subscription = skeleton.joint_angles().subscribe(print_joint_angles)
        pose_subscription = skeleton.pose().subscribe(print_pose)
        glove.start()

        try:
            glove.run_forever()
        except KeyboardInterrupt:
            print("\nstopping")
        finally:
            del pose_subscription
            del angle_subscription
    finally:
        glove.close()


if __name__ == "__main__":
    main()
