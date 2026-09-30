from __future__ import annotations

import argparse

from qnbot_sdk import CompositeExoGloveConfig, Sdk, SerialConnection, Side
from qnbot_sdk.exo import ExoConfig
from qnbot_sdk.glove import GloveConfig


def main() -> None:
    arguments = argparse.ArgumentParser(description="Run a short composite lifecycle")
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
        sdk.start()
        sdk.update()
        print(
            f"started Left Glove={left_glove_device.source_id} "
            f"Right Glove={right_glove_device.source_id} Exo={exo_device.source_id}"
        )
        print(f"Left Glove pose={left_glove_device.pose().latest()}")
        print(f"Right Glove pose={right_glove_device.pose().latest()}")
        print(f"Exo telemetry={exo_device.telemetry().latest()}")
        sdk.stop()
        print("stopped")
    finally:
        sdk.close()
        print("closed")


if __name__ == "__main__":
    main()
