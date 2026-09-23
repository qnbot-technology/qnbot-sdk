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
    arguments = argparse.ArgumentParser(description="Run a short composite lifecycle")
    arguments.add_argument("--port", required=True, help="Shared serial port")
    options = arguments.parse_args()

    sdk = create_sdk(port=options.port)
    try:
        glove_domain = sdk.glove()
        exo_domain = sdk.exo()
        glove = glove_domain.device(GLOVE_NAME)
        exo = exo_domain.device(EXO_NAME)
        glove_domain.start()
        exo_domain.start()
        glove_domain.update()
        exo_domain.update()
        print(f"started Glove={glove.source_id} Exo={exo.source_id}")
        print(f"Glove pose={glove.pose().latest()}")
        print(f"Exo telemetry={exo.telemetry().latest()}")
        glove_domain.stop()
        exo_domain.stop()
        print("stopped")
    finally:
        glove_domain.close()
        exo_domain.close()
        print("closed")


if __name__ == "__main__":
    main()
