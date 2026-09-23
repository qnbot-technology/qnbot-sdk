from __future__ import annotations

import argparse

from qnbot_sdk import (
    CompositeExoGloveConfig,
    DeviceStatus,
    Sample,
    Sdk,
    SerialConnection,
)
from qnbot_sdk.exo import ExoConfig, ExoTelemetry
from qnbot_sdk.glove import GloveConfig, GlovePose

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


def print_glove(sample: Sample[GlovePose]) -> None:
    print(
        f"glove pose sequence={sample.sequence} hand={sample.value.payload.hand_side}"
    )


def print_exo(sample: Sample[ExoTelemetry]) -> None:
    print(f"exo telemetry sequence={sample.sequence} quality={sample.value.quality}")


def print_status(sample: Sample[DeviceStatus]) -> None:
    print(f"exo status sequence={sample.sequence} value={sample.value}")


def print_glove_status(sample: Sample[DeviceStatus]) -> None:
    print(f"glove status sequence={sample.sequence} value={sample.value}")


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Read combined telemetry and status"
    )
    arguments.add_argument("--port", required=True, help="Shared serial port")
    options = arguments.parse_args()

    sdk = create_sdk(port=options.port)
    try:
        glove_domain = sdk.glove()
        exo_domain = sdk.exo()
        glove = glove_domain.device(GLOVE_NAME)
        exo = exo_domain.device(EXO_NAME)
        glove.pose().subscribe(print_glove)
        glove.status().subscribe(print_glove_status)
        exo.telemetry().subscribe(print_exo)
        exo.status().subscribe(print_status)
        glove_domain.start()
        exo_domain.start()
        print("running; press Ctrl+C to stop", flush=True)
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
