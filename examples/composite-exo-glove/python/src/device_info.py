from __future__ import annotations

import argparse

from qnbot_sdk import CompositeExoGloveConfig, Sdk, SerialConnection, Side
from qnbot_sdk.exo import ExoConfig
from qnbot_sdk.glove import GloveConfig


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Read the Exo info and show the Composite Glove source"
    )
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
        left_glove_device = glove.device(side=Side.LEFT)
        right_glove_device = glove.device(side=Side.RIGHT)
        exo_device = exo.device()
        exo_info = exo_device.get_device_info()
        print(f"Left Glove source={left_glove_device.source_id}")
        print(f"Right Glove source={right_glove_device.source_id}")
        print(
            f"Exo product={exo_info.product.name} protocol="
            f"{exo_info.protocol_version.major}.{exo_info.protocol_version.minor} "
            f"serial={exo_info.serial_number} hardware={exo_info.hardware_name}"
        )
    finally:
        sdk.close()


if __name__ == "__main__":
    main()
