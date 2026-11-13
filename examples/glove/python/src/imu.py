from __future__ import annotations

import argparse

from qnbot_sdk import Sample, Sdk, SerialConnection, Side, SubscriptionConfig
from qnbot_sdk.glove import (
    GloveConfig,
    GloveImu,
    GloveImuRawCounts,
    GlovePose,
)


def print_imu(sample: Sample[GloveImu]) -> None:
    for imu in sample.value.payload.imus:
        if isinstance(imu.values, GloveImuRawCounts):
            values = f"raw_counts gyro={imu.values.gyroscope_raw} accel={imu.values.accelerometer_raw}"
        else:
            values = f"gyro_mdps={imu.values.gyro_mdps} accel_mg={imu.values.accel_mg}"
        print(
            f"imu id={imu.id} location={imu.location.value} valid={imu.valid} fresh={imu.fresh} {values}"
        )


def print_pose(sample: Sample[GlovePose]) -> None:
    hand = sample.value.payload.hand
    if hand is None:
        print("hand absent")
    else:
        print(
            f"hand state={'fresh' if hand.fresh else 'held'} "
            f"timestamp_ms={hand.timestamp_ms} received_ms={hand.received_monotonic_ms} "
            f"fingertips={len(hand.fingertips)}"
        )
    wrist = sample.value.payload.wrist
    if wrist is None:
        print("wrist absent")
    else:
        print(
            f"wrist state={'fresh' if wrist.fresh else 'held'} valid={wrist.valid} "
            f"timestamp_ms={wrist.timestamp_ms} received_ms={wrist.received_monotonic_ms} "
            f"position_m={wrist.position} quaternion_xyzw={wrist.quaternion_xyzw}"
        )


def main() -> None:
    arguments = argparse.ArgumentParser(description="Read glove IMU raw counts")
    arguments.add_argument(
        "--port", required=True, help="Serial port for the glove or wireless Receiver"
    )
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
    device = glove.device()

    try:
        device.imu().with_config(SubscriptionConfig.fifo(512)).subscribe(print_imu)
        device.pose().subscribe(print_pose)
        glove.start()
        print("running; press Ctrl+C to stop")
        glove.run_forever()
    finally:
        glove.close()


if __name__ == "__main__":
    main()
