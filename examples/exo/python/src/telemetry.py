from __future__ import annotations

import argparse

from qnbot_sdk import DeviceStatus, Sample, Sdk, SerialConnection
from qnbot_sdk.exo import ExoConfig, ExoTelemetry


def create_sdk(port: str, name: str) -> Sdk:
    return Sdk(devices=(ExoConfig(name=name, connection=SerialConnection(port=port)),))


def print_telemetry(sample: Sample[ExoTelemetry]) -> None:
    payload = sample.value.payload
    print(
        f"telemetry sequence={sample.sequence} "
        f"left_arm={payload.left_arm.encoder_counts} "
        f"right_arm={payload.right_arm.encoder_counts} "
        f"quality={sample.value.quality}"
    )


def print_status(sample: Sample[DeviceStatus]) -> None:
    print(f"status sequence={sample.sequence} value={sample.value}")


def main() -> None:
    arguments = argparse.ArgumentParser(description="Read Exo telemetry and status")
    arguments.add_argument("--port", required=True, help="Serial port for the device")
    arguments.add_argument("--name", default="primary", help="Logical device name")
    options = arguments.parse_args()

    sdk = create_sdk(options.port, options.name)
    exo = sdk.exo()
    device = exo.device(options.name)
    device.telemetry().subscribe(print_telemetry)
    device.status().subscribe(print_status)

    exo.start()
    print("running; press Ctrl+C to stop", flush=True)
    exo.run_forever()


if __name__ == "__main__":
    main()
