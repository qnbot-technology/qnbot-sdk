from __future__ import annotations

import argparse

from qnbot_sdk import CompositeExoGloveConfig, Sdk, SerialConnection
from qnbot_sdk.exo import ExoConfig
from qnbot_sdk.glove import GloveConfig

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


def main() -> None:
    arguments = argparse.ArgumentParser(description="Read both composite member infos")
    arguments.add_argument("--port", required=True, help="Shared serial port")
    options = arguments.parse_args()

    sdk = create_sdk(port=options.port)
    try:
        glove_domain = sdk.glove()
        exo_domain = sdk.exo()
        glove = glove_domain.device(GLOVE_NAME)
        exo = exo_domain.device(EXO_NAME)
        glove_info = glove.get_device_info()
        exo_info = exo.get_device_info()
        print(
            f"Glove sn={glove_info.canonical_sn} model={glove_info.model} "
            f"hand={glove_info.hand.value} firmware={glove_info.firmware_version}"
        )
        print(
            f"Exo product={exo_info.product.name} protocol="
            f"{exo_info.protocol_version.major}.{exo_info.protocol_version.minor} "
            f"serial={exo_info.serial_number} hardware={exo_info.hardware_name}"
        )
    finally:
        glove_domain.close()
        exo_domain.close()


if __name__ == "__main__":
    main()
