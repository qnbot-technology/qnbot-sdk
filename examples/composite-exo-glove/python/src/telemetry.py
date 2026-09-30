from __future__ import annotations

import argparse

from qnbot_sdk import (
    CompositeExoGloveConfig,
    Sample,
    Sdk,
    SerialConnection,
    Side,
)
from qnbot_sdk.exo import ExoConfig, ExoStatus, ExoTelemetry
from qnbot_sdk.glove import GloveConfig, GlovePose, GloveStatus


def print_glove(label: str, sample: Sample[GlovePose]) -> None:
    print(
        f"{label} glove pose sequence={sample.sequence} "
        f"hand={sample.value.payload.hand_side}"
    )


def print_exo(sample: Sample[ExoTelemetry]) -> None:
    print(f"exo telemetry sequence={sample.sequence} quality={sample.value.quality}")


def print_status(sample: Sample[ExoStatus]) -> None:
    print(f"exo status sequence={sample.sequence} value={sample.value}")


def print_glove_status(label: str, sample: Sample[GloveStatus]) -> None:
    print(f"{label} glove status sequence={sample.sequence} value={sample.value}")


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Read combined telemetry and status"
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
        left_glove_device.pose().subscribe(lambda sample: print_glove("left", sample))
        right_glove_device.pose().subscribe(lambda sample: print_glove("right", sample))
        left_glove_device.status().subscribe(
            lambda sample: print_glove_status("left", sample)
        )
        right_glove_device.status().subscribe(
            lambda sample: print_glove_status("right", sample)
        )
        exo_device.telemetry().subscribe(print_exo)
        exo_device.status().subscribe(print_status)
        sdk.start()
        print("running; press Ctrl+C to stop")
        # The root SDK lifecycle drives both member domains.
        sdk.run_forever()
    except KeyboardInterrupt:
        sdk.stop()
    finally:
        sdk.close()


if __name__ == "__main__":
    main()
