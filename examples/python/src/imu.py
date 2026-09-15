from __future__ import annotations

import argparse

from qnbot_sdk import Sample, Sdk, SerialConnection, Side
from qnbot_sdk.glove import GloveConfig, GloveImu


def print_imu(sample: Sample[GloveImu]) -> None:
    imu = sample.value.payload
    print(
        f"imu sequence={sample.sequence} valid={imu.valid} "
        f"gyroscope_raw={imu.gyroscope_raw} "
        f"accelerometer_raw={imu.accelerometer_raw}"
    )


def main() -> None:
    arguments = argparse.ArgumentParser(description="Read wired glove IMU raw counts")
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
    imu = glove.device().imu()
    _imu_subscription = imu.subscribe(print_imu)

    glove.start()
    print("running; press Ctrl+C to stop", flush=True)
    glove.run_forever()


if __name__ == "__main__":
    main()
