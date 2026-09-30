from __future__ import annotations

import argparse
import time

from qnbot_sdk import Sdk, SerialConnection
from qnbot_sdk.exo import ExoConfig, ExoHaptics


def main() -> None:
    arguments = argparse.ArgumentParser(description="Set and clear Exo haptics")
    arguments.add_argument("--port", required=True, help="Serial port for the device")
    arguments.add_argument("--left", type=int, default=60)
    arguments.add_argument("--right", type=int, default=60)
    arguments.add_argument("--hold", type=float, default=1.0)
    options = arguments.parse_args()

    sdk = Sdk(
        devices=(ExoConfig(connection=SerialConnection(port=options.port)),)
    )
    exo = sdk.exo()
    device = exo.device()
    if not device.get_device_info().capabilities.haptics:
        print("this device reports no haptics")
        exo.close()
        return

    exo.start()
    haptics = device.haptics()
    haptics.set(ExoHaptics(left=options.left, right=options.right))
    print(f"set left={options.left} right={options.right}")
    time.sleep(options.hold)

    haptics.clear()
    print("haptics cleared")
    exo.stop()
    exo.close()


if __name__ == "__main__":
    main()
