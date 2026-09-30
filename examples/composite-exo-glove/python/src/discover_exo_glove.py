from __future__ import annotations

from qnbot_sdk import CompositeExoGloveConfig, QnBotError, Sdk, SerialConnection, Side
from qnbot_sdk.exo import ExoConfig
from qnbot_sdk.glove import GloveConfig


def main() -> None:
    sdk = Sdk(
        devices=(
            CompositeExoGloveConfig(
                connection=SerialConnection(auto_discover=True),
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
        print(f"Left Glove discovered source={left_glove_device.source_id}")
        print(f"Right Glove discovered source={right_glove_device.source_id}")
        try:
            exo_info = exo_device.get_device_info()
            print(
                "Exo discovered "
                f"product={exo_info.product.name} sn={exo_info.serial_number} "
                f"hardware={exo_info.hardware_name}"
            )
        except QnBotError as error:
            print(f"Exo discovery error: {error}")
    finally:
        sdk.close()


if __name__ == "__main__":
    main()
