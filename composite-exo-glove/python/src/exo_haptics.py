from __future__ import annotations

import argparse
import time

from qnbot_sdk import CompositeExoGloveConfig, Sdk, SerialConnection
from qnbot_sdk.exo import ExoConfig, ExoHaptics
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
    arguments = argparse.ArgumentParser(description="Set Exo haptics in a composite")
    arguments.add_argument("--port", required=True, help="Shared serial port")
    arguments.add_argument("--left", type=int, default=60)
    arguments.add_argument("--right", type=int, default=60)
    arguments.add_argument("--hold", type=float, default=1.0)
    options = arguments.parse_args()

    sdk = create_sdk(port=options.port)
    try:
        exo_domain = sdk.exo()
        exo = exo_domain.device(EXO_NAME)
        if not exo.get_device_info().capabilities.haptics:
            print("this Exo reports no haptics", flush=True)
            return
        exo_domain.start()
        haptics = exo.haptics()
        haptics.set(ExoHaptics(left=options.left, right=options.right))
        print(f"set Exo haptics left={options.left} right={options.right}", flush=True)
        time.sleep(options.hold)
        haptics.clear()
        print("Exo haptics cleared", flush=True)
        exo_domain.stop()
    finally:
        exo_domain.close()


if __name__ == "__main__":
    main()
