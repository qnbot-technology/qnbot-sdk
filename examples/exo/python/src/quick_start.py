from __future__ import annotations

from qnbot_sdk import Sample, Sdk
from qnbot_sdk.exo import ExoConfig, ExoTelemetry


def print_telemetry(sample: Sample[ExoTelemetry]) -> None:
    print(
        f"telemetry sequence={sample.sequence} "
        f"left_arm={sample.value.payload.left_arm.joint_positions_rad}"
    )


def main() -> None:
    sdk = Sdk(devices=(ExoConfig(),))
    exo = sdk.exo()
    device = exo.device()
    device.telemetry().subscribe(print_telemetry)

    exo.start()
    print("running; press Ctrl+C to stop", flush=True)
    exo.run_forever()


if __name__ == "__main__":
    main()
