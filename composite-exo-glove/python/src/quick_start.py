from __future__ import annotations

import argparse

from qnbot_sdk import CompositeExoGloveConfig, Sdk, SerialConnection
from qnbot_sdk.exo import ExoConfig
from qnbot_sdk.glove import GloveConfig

EXO_NAME = "exo"
GLOVE_NAME = "glove"


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Start a combined Exo and Glove SDK"
    )
    arguments.add_argument("--port", required=True, help="Shared serial port")
    options = arguments.parse_args()

    sdk = Sdk(
        devices=(
            CompositeExoGloveConfig(
                connection=SerialConnection(port=options.port),
                devices=(GloveConfig(name=GLOVE_NAME), ExoConfig(name=EXO_NAME)),
            ),
        )
    )
    try:
        glove_domain = sdk.glove()
        exo_domain = sdk.exo()
        glove = glove_domain.device(GLOVE_NAME)
        exo = exo_domain.device(EXO_NAME)
        glove_domain.start()
        exo_domain.start()
        print(f"started Glove={glove.source_id} Exo={exo.source_id}", flush=True)
        print("press Ctrl+C to stop", flush=True)
        # The aggregate runner is required to drive both member domains.
        sdk.run_forever()
    except KeyboardInterrupt:
        glove_domain.stop()
        exo_domain.stop()
    finally:
        glove_domain.close()
        exo_domain.close()


if __name__ == "__main__":
    main()
