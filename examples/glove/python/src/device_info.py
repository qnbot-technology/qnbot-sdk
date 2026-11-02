from __future__ import annotations

import argparse

from qnbot_sdk import Sdk, SerialConnection, Side
from qnbot_sdk.glove import GloveConfig


def main() -> None:
    parser = argparse.ArgumentParser(description="Read glove device information")
    parser.add_argument("--port", help="Optional serial port for a glove or Receiver")
    parser.add_argument("--side", choices=("left", "right"), help="Glove side")
    options = parser.parse_args()
    config = GloveConfig(
        side=Side(options.side) if options.side else None,
        connection=SerialConnection(port=options.port)
        if options.port
        else SerialConnection(),
    )
    sdk = Sdk(devices=(config,))
    glove = sdk.glove()
    try:
        device = glove.device()
        info = device.get_device_info()
        print(
            f"source={device.source_id} canonical_sn={info.canonical_sn} "
            f"device_type={info.device_type or 'unknown'} model={info.model or 'unknown'} "
            f"firmware_version={info.firmware_version or 'unknown'} hand={info.hand.value} "
            f"hand_source={info.hand_source or 'unknown'} "
            f"transport_type={info.transport_type.value}"
        )
    finally:
        glove.close()


if __name__ == "__main__":
    main()
