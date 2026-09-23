from __future__ import annotations

import argparse

from qnbot_sdk import CompositeExoGloveConfig, Sample, Sdk, SerialConnection
from qnbot_sdk.exo import ExoConfig, ExoTelemetry
from qnbot_sdk.glove import GloveConfig, GloveImu

EXO_NAME = "exo"
GLOVE_NAME = "glove"


def create_sdk(port: str) -> Sdk:
    return Sdk(
        devices=(
            CompositeExoGloveConfig(
                connection=SerialConnection(port=port),
                devices=(GloveConfig(name=GLOVE_NAME), ExoConfig(name=EXO_NAME)),
            ),
        )
    )


def print_glove_imu(sample: Sample[GloveImu]) -> None:
    imu = sample.value.payload
    print(
        f"glove imu sequence={sample.sequence} valid={imu.valid} "
        f"gyro={imu.gyroscope_raw} accel={imu.accelerometer_raw}"
    )


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
    arguments = argparse.ArgumentParser(description="Read Glove and Exo IMU data")
    arguments.add_argument("--port", required=True, help="Shared serial port")
    options = arguments.parse_args()

    sdk = create_sdk(port=options.port)
    try:
        glove_domain = sdk.glove()
        exo_domain = sdk.exo()
        glove = glove_domain.device(GLOVE_NAME)
        exo = exo_domain.device(EXO_NAME)
        glove.imu().subscribe(print_glove_imu)
        exo.telemetry().subscribe(print_exo_imu)
        glove_domain.start()
        exo_domain.start()
        print("running; press Ctrl+C to stop", flush=True)
        # The aggregate runner is required to drive both member domains.
        sdk.run_forever()
    except KeyboardInterrupt:
        glove_domain.stop()
        exo_domain.stop()
    finally:
        glove_domain.close()
        exo_domain.close()


if __name__ == "__main__":
    main()
