from __future__ import annotations

import argparse

from qnbot_sdk import Sample, Sdk, SerialConnection
from qnbot_sdk.exo import ExoConfig, ExoTelemetry


def print_handsets(sample: Sample[ExoTelemetry]) -> None:
    payload = sample.value.payload
    print(
        f"handsets sequence={sample.sequence} "
        f"left=({payload.left_handset.axis_x_raw}, "
        f"{payload.left_handset.axis_y_raw}, "
        f"trigger={payload.left_handset.trigger_raw}, "
        f"buttons=0x{payload.left_handset.button_mask:04x}) "
        f"right=({payload.right_handset.axis_x_raw}, "
        f"{payload.right_handset.axis_y_raw}, "
        f"trigger={payload.right_handset.trigger_raw}, "
        f"buttons=0x{payload.right_handset.button_mask:04x})"
    )


def main() -> None:
    arguments = argparse.ArgumentParser(description="Read Exo handset states")
    arguments.add_argument("--port", required=True, help="Serial port for the device")
    options = arguments.parse_args()

    sdk = Sdk(
        devices=(ExoConfig(connection=SerialConnection(port=options.port)),)
    )
    exo = sdk.exo()
    device = exo.device()
    if not device.get_device_info().capabilities.handsets:
        print("this device reports no handset")
        exo.close()
        return

    device.telemetry().subscribe(print_handsets)
    exo.start()
    print("running; press Ctrl+C to stop")
    exo.run_forever()


if __name__ == "__main__":
    main()
