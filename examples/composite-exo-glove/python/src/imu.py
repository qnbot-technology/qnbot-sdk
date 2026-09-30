from __future__ import annotations

import argparse

from qnbot_sdk import CompositeExoGloveConfig, Sample, Sdk, SerialConnection, Side
from qnbot_sdk.exo import ExoConfig, ExoTelemetry
from qnbot_sdk.glove import GloveConfig


def print_exo_imu(sample: Sample[ExoTelemetry]) -> None:
    payload = sample.value.payload
    for label, imu in (("torso", payload.torso_imu), ("extra", payload.extra_imu)):
        if imu is not None:
            print(
                f"exo {label} imu sequence={sample.sequence} "
                f"acceleration={imu.acceleration_mps2} "
                f"angular_velocity={imu.angular_velocity_rad_s}"
            )


def main() -> None:
    arguments = argparse.ArgumentParser(description="Read Exo IMU data on a composite link")
    arguments.add_argument("--port", required=True, help="Shared serial port")
    options = arguments.parse_args()

    sdk = Sdk(
        devices=(
            CompositeExoGloveConfig(
                connection=SerialConnection(port=options.port),
                devices=(
                    GloveConfig(side=Side.LEFT),
                    GloveConfig(side=Side.RIGHT),
                    ExoConfig(),
                ),
            ),
        )
    )
    try:
        glove = sdk.glove()
        exo = sdk.exo()
        exo_device = exo.device()
        print(
            "Glove raw IMU (Telemetry.ImuRawSnapshot, 0x10/0x81) is not exposed "
            "on the composite CDC path; reading Exo IMU only",
        )
        exo_device.telemetry().subscribe(print_exo_imu)
        sdk.start()
        print("running; press Ctrl+C to stop")
        # The aggregate runner is required to drive both member domains.
        sdk.run_forever()
    except KeyboardInterrupt:
        sdk.stop()
    finally:
        sdk.close()


if __name__ == "__main__":
    main()
