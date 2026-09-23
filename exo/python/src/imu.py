from __future__ import annotations

import argparse

from qnbot_sdk import Sample, Sdk, SerialConnection
from qnbot_sdk.exo import ExoConfig, ExoTelemetry


def create_sdk(port: str, name: str) -> Sdk:
    return Sdk(devices=(ExoConfig(name=name, connection=SerialConnection(port=port)),))


def print_imus(sample: Sample[ExoTelemetry]) -> None:
    payload = sample.value.payload
    for label, imu in (("torso", payload.torso_imu), ("extra", payload.extra_imu)):
        if imu is None:
            continue
        print(
            f"{label} imu sequence={sample.sequence} "
            f"acceleration={imu.acceleration_mps2} "
            f"angular_velocity={imu.angular_velocity_rad_s} "
            f"orientation={imu.orientation_xyzw}"
        )


def main() -> None:
    arguments = argparse.ArgumentParser(description="Read Exo IMU payloads")
    arguments.add_argument("--port", required=True, help="Serial port for the device")
    arguments.add_argument("--name", default="primary", help="Logical device name")
    options = arguments.parse_args()

    sdk = create_sdk(options.port, options.name)
    exo = sdk.exo()
    device = exo.device(options.name)
    if not device.get_device_info().capabilities.imus:
        print("this device reports no IMU", flush=True)
        exo.close()
        return

    device.telemetry().subscribe(print_imus)
    exo.start()
    print("running; press Ctrl+C to stop", flush=True)
    exo.run_forever()


if __name__ == "__main__":
    main()
