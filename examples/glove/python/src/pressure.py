from __future__ import annotations

import argparse

from qnbot_sdk import Sample, Sdk, SerialConnection, SubscriptionConfig
from qnbot_sdk.glove import GloveConfig, GlovePressure


def print_pressure(sample: Sample[GlovePressure]) -> None:
    readings = sample.value.readings
    print(
        f"pressure sequence={sample.sequence} points={len(readings)} "
        f"first_raw={readings[0]}"
    )


def main() -> None:
    arguments = argparse.ArgumentParser(description="Read glove pressure")
    arguments.add_argument("--port", required=True, help="Glove serial port")
    options = arguments.parse_args()
    sdk = Sdk(devices=(GloveConfig(connection=SerialConnection(port=options.port)),))
    glove = sdk.glove()
    device = glove.device()
    device.pressure().with_config(SubscriptionConfig.latest(max_rate_hz=1)).subscribe(
        print_pressure
    )

    try:
        glove.start()
        print("running; press Ctrl+C to stop", flush=True)
        glove.run_forever()
    finally:
        glove.close()


if __name__ == "__main__":
    main()
