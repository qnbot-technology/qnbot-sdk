from __future__ import annotations

from qnbot_sdk.glove import discover_gloves


def main() -> None:
    discovered = discover_gloves()
    print(f"discovered records={len(discovered)}")
    for device in discovered:
        hand = device.hand.value if device.hand is not None else "unknown"
        sn = device.sn if device.sn is not None else "unknown"
        status = "error" if device.error is not None else "ok"
        information = f"transport_type={device.transport_type.value} port={device.port} hand={hand} sn={sn} status={status}"
        if device.error is not None:
            print(
                f"{information} error={device.error.code.value} "
                f"message={device.error.message}"
            )
        else:
            print(information)


if __name__ == "__main__":
    main()
