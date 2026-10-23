from __future__ import annotations

import argparse

from qnbot_sdk import (
    DeviceSelector,
    Sample,
    Sdk,
    Side,
    TargetAlgorithm,
    TargetConfig,
)
from qnbot_sdk.glove import GloveConfig, GloveReceiverConfig, HandJointCommand


def print_output(side: str, sample: Sample[HandJointCommand]) -> None:
    print(
        f"{side} retargeting sequence={sample.sequence} "
        f"target={sample.value.target} joints={sample.value.joints}"
    )


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Read target hand output from left and right wireless gloves"
    )
    arguments.add_argument("--package-id", required=True)
    options = arguments.parse_args()

    sdk = Sdk(
        devices=(
            GloveReceiverConfig(
                devices=(
                    GloveConfig(side=Side.LEFT),
                    GloveConfig(side=Side.RIGHT),
                ),
            ),
        ),
        targets=tuple(
            TargetConfig(
                side=side,
                source=DeviceSelector(type="glove", side=side),
                algorithms=(TargetAlgorithm(id=options.package_id),),
            )
            for side in (Side.LEFT, Side.RIGHT)
        ),
    )
    glove = sdk.glove()
    try:
        for side in (Side.LEFT, Side.RIGHT):
            device = glove.device(side=side)
            device.output().subscribe(
                lambda sample, side=side: print_output(side.value, sample)
            )
        glove.start()
        print("running; press Ctrl+C to stop")
        glove.run_forever()
    finally:
        glove.close()


if __name__ == "__main__":
    main()
