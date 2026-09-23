from __future__ import annotations

import argparse
import time

from qnbot_sdk import Health, Sdk, SerialConnection
from qnbot_sdk.exo import ExoConfig


def create_sdk(first_port: str, second_port: str) -> Sdk:
    return Sdk(
        devices=(
            ExoConfig(name="first", connection=SerialConnection(port=first_port)),
            ExoConfig(name="second", connection=SerialConnection(port=second_port)),
        )
    )


def print_health(label: str, health: Health) -> None:
    print(
        f"{label} health ok={health.ok} warnings={health.warning_count} "
        f"errors={health.error_count}"
    )


def main() -> None:
    arguments = argparse.ArgumentParser(description="Manage two Exo devices apart")
    arguments.add_argument("--first-port", required=True)
    arguments.add_argument("--second-port", required=True)
    arguments.add_argument("--seconds", type=float, default=3.0)
    options = arguments.parse_args()

    sdk = create_sdk(options.first_port, options.second_port)
    exo = sdk.exo()
    first = exo.device("first")
    second = exo.device("second")
    first.start()
    second.start()
    print_health("first", first.health())
    print_health("second", second.health())
    time.sleep(options.seconds)

    second.stop()
    print("second device stopped; first remains active", flush=True)
    print_health("first", first.health())
    latest = first.telemetry().latest()
    if latest is not None:
        print(
            f"first telemetry sequence={latest.sequence} "
            f"left_arm={latest.value.payload.left_arm.encoder_counts}"
        )

    first.stop()
    first.close()
    second.close()


if __name__ == "__main__":
    main()
