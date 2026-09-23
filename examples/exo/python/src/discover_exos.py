from __future__ import annotations

from qnbot_sdk.exo import discover_exos


def main() -> None:
    for device in discover_exos():
        if device.error is not None:
            print(
                f"port={device.port} error={device.error.code.value} "
                f"message={device.error.message}"
            )
            continue
        info = device.device_info
        print(
            f"port={device.port} product={info.product.name} "
            f"protocol={info.protocol_version.major}.{info.protocol_version.minor} "
            f"sn={info.serial_number} name={info.hardware_name}"
        )


if __name__ == "__main__":
    main()
