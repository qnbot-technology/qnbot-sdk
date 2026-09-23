from __future__ import annotations

import argparse

from qnbot_sdk import Sdk, SerialConnection
from qnbot_sdk.exo import ExoConfig, ExoDeviceInfo


def create_sdk(port: str, name: str) -> Sdk:
    return Sdk(devices=(ExoConfig(name=name, connection=SerialConnection(port=port)),))


def print_info(info: ExoDeviceInfo) -> None:
    print(
        f"product={info.product.name} "
        f"protocol={info.protocol_version.major}.{info.protocol_version.minor} "
        f"firmware={info.firmware} sn={info.serial_number} name={info.hardware_name}"
    )
    print(
        "components "
        f"encoders={info.components.encoders.available}/"
        f"{info.components.encoders.total_slots} "
        f"handsets={info.components.handsets.available}/"
        f"{info.components.handsets.total_slots} "
        f"imus={info.components.imus.available}/{info.components.imus.total_slots}"
    )
    capabilities = info.capabilities
    print(
        "capabilities "
        f"telemetry={capabilities.telemetry} encoders={capabilities.encoders} "
        f"handsets={capabilities.handsets} imus={capabilities.imus} "
        f"haptics={capabilities.haptics} wireless={capabilities.wireless}"
    )


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Read Exo device information before start"
    )
    arguments.add_argument("--port", required=True, help="Serial port for the device")
    arguments.add_argument("--name", default="primary", help="Logical device name")
    options = arguments.parse_args()

    sdk = create_sdk(options.port, options.name)
    exo = sdk.exo()
    device = exo.device(options.name)
    print_info(device.get_device_info())
    exo.close()


if __name__ == "__main__":
    main()
