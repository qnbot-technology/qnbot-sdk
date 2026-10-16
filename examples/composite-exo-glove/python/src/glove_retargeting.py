from __future__ import annotations

import argparse

from qnbot_sdk import (
    CompositeExoGloveConfig,
    DeviceSelector,
    Sample,
    Sdk,
    SerialConnection,
    Side,
    TargetAlgorithm,
    TargetConfig,
)
from qnbot_sdk.exo import ExoConfig
from qnbot_sdk.glove import GloveConfig, HandJointCommand


def print_output(label: str, sample: Sample[HandJointCommand]) -> None:
    print(
        f"{label} retargeting sequence={sample.sequence} "
        f"target={sample.value.target} joints={sample.value.joints}"
    )


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Read retargeted output from a composite Exo and Glove device"
    )
    arguments.add_argument("--port", required=True, help="Shared serial port")
    arguments.add_argument(
        "--package-id",
        required=True,
        help="Installed target hand algorithm package ID",
    )
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
        ),
        targets=(
            TargetConfig(
                side=Side.LEFT,
                source=DeviceSelector(type="glove", side=Side.LEFT),
                algorithms=(TargetAlgorithm(id=options.package_id),),
            ),
            TargetConfig(
                side=Side.RIGHT,
                source=DeviceSelector(type="glove", side=Side.RIGHT),
                algorithms=(TargetAlgorithm(id=options.package_id),),
            ),
        ),
    )
    glove = sdk.glove()
    left = glove.device(side=Side.LEFT)
    right = glove.device(side=Side.RIGHT)
    try:
        left.output().subscribe(lambda sample: print_output("left", sample))
        right.output().subscribe(lambda sample: print_output("right", sample))
        sdk.start()
        print("running; press Ctrl+C to stop")
        sdk.run_forever()
    except KeyboardInterrupt:
        sdk.stop()
    finally:
        sdk.close()


if __name__ == "__main__":
    main()
