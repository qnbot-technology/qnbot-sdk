from __future__ import annotations

from qnbot_sdk import CompositeExoGloveConfig, QnBotError, Sdk, SerialConnection
from qnbot_sdk.exo import ExoConfig
from qnbot_sdk.glove import GloveConfig

EXO_NAME = "exo"
GLOVE_NAME = "glove"


def main() -> None:
    sdk = Sdk(
        devices=(
            CompositeExoGloveConfig(
                connection=SerialConnection(auto_discover=True),
                devices=(GloveConfig(name=GLOVE_NAME), ExoConfig(name=EXO_NAME)),
            ),
        )
    )
    try:
        glove_domain = sdk.glove()
        exo_domain = sdk.exo()
        glove = glove_domain.device(GLOVE_NAME)
        exo = exo_domain.device(EXO_NAME)
        try:
            glove_info = glove.get_device_info()
            print(
                "Glove discovered "
                f"sn={glove_info.canonical_sn} model={glove_info.model} "
                f"hand={glove_info.hand.value} firmware={glove_info.firmware_version}"
            )
        except QnBotError as error:
            print(f"Glove discovery error: {error}")
        try:
            exo_info = exo.get_device_info()
            print(
                "Exo discovered "
                f"product={exo_info.product.name} sn={exo_info.serial_number} "
                f"hardware={exo_info.hardware_name}"
            )
        except QnBotError as error:
            print(f"Exo discovery error: {error}")
    finally:
        glove_domain.close()
        exo_domain.close()


if __name__ == "__main__":
    main()
