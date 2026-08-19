from __future__ import annotations

from qnbot_sdk import SerialConnection
from qnbot_sdk.glove import GloveConfig, discover_gloves


def main() -> None:
    discovered = discover_gloves()
    for device in discovered:
        if device.error is not None:
            print(
                f"port={device.port} error={device.error.code.value} "
                f"message={device.error.message}"
            )
            continue
        print(
            f"port={device.port} hand={device.hand.value} "
            f"sn={device.sn} activated={device.activated}"
        )

    selected = next((device for device in discovered if device.error is None), None)
    if selected is None or selected.hand is None:
        return

    config = GloveConfig(
        side=selected.hand,
        connection=SerialConnection(port=selected.port),
    )
    print(f"selected port={config.connection.port} hand={config.side.value}")


if __name__ == "__main__":
    main()
