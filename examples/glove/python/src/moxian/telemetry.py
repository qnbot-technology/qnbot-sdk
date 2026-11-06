from __future__ import annotations

import argparse

from qnbot_sdk import Sample, Sdk, SerialConnection, SubscriptionConfig
from qnbot_sdk.glove import (
    GloveConfig,
    GloveFinger,
    GloveImu,
    GlovePose,
    GlovePressure,
    HandJointCommand,
    HandSkeletonPose,
)


def print_pose(sample: Sample[GlovePose]) -> None:
    tip = sample.value.payload.fingertip_local[GloveFinger.INDEX]
    print(f"pose sequence={sample.sequence} index_position={tip.position}")


def print_imu(sample: Sample[GloveImu]) -> None:
    imu = sample.value.payload
    print(
        f"imu sequence={sample.sequence} valid={imu.valid} "
        f"gyroscope_raw={imu.gyroscope_raw} accelerometer_raw={imu.accelerometer_raw}"
    )


def print_joints(sample: Sample[HandJointCommand]) -> None:
    joints = sample.value.joints
    joint, angle = next(iter(joints.items()))
    print(
        f"joint_angles sequence={sample.sequence} joints={len(joints)} "
        f"first_joint={joint} angle={angle}"
    )


def print_skeleton_pose(sample: Sample[HandSkeletonPose]) -> None:
    pose = sample.value
    index_tip = pose.names.index("index_tip")
    print(
        f"skeleton_pose sequence={sample.sequence} frame={pose.coordinate_frame} "
        f"node=index_tip position={pose.positions_local[index_tip]} "
        f"quaternion_xyzw={pose.orientations_local_xyzw[index_tip]}"
    )


def print_pressure(sample: Sample[GlovePressure]) -> None:
    readings = sample.value.readings
    print(
        f"pressure sequence={sample.sequence} points={len(readings)} "
        f"first_raw={readings[0]}"
    )


def main() -> None:
    arguments = argparse.ArgumentParser(description="Read Moxian Glove telemetry")
    arguments.add_argument("--port", required=True, help="Glove serial port")
    options = arguments.parse_args()
    sdk = Sdk(devices=(GloveConfig(connection=SerialConnection(port=options.port)),))
    glove = sdk.glove()
    try:
        device = glove.device()
        skeleton = device.skeleton()
        display = SubscriptionConfig.latest(max_rate_hz=1)
        _subscriptions = (
            device.pose().with_config(display).subscribe(print_pose),
            device.imu().with_config(display).subscribe(print_imu),
            skeleton.joint_angles().with_config(display).subscribe(print_joints),
            skeleton.pose().with_config(display).subscribe(print_skeleton_pose),
            device.pressure().with_config(display).subscribe(print_pressure),
        )

        glove.start()
        print("running; press Ctrl+C to stop", flush=True)
        glove.run_forever()
    finally:
        glove.close()


if __name__ == "__main__":
    main()
