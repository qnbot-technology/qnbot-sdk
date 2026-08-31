from __future__ import annotations

import argparse

from qnbot_sdk import (
    Sample,
    Sdk,
    TargetAlgorithm,
    TargetConfig,
)
from qnbot_sdk.glove import GloveConfig, GlovePose, HandJointCommand


def print_pose(sample: Sample[GlovePose]) -> None:
    print(
        f"pose sequence={sample.sequence} "
        f"fingertips={sample.value.payload.fingertip_local}"
    )


def print_output(sample: Sample[HandJointCommand]) -> None:
    print(
        f"retargeting sequence={sample.sequence} "
        f"target={sample.value.target} joints={sample.value.joints}"
    )


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Run the minimal target-output workflow"
    )
    arguments.add_argument("--package-id", required=True)
    options = arguments.parse_args()
    sdk = Sdk(
        devices=(GloveConfig(),),
        targets=(
            TargetConfig(
                algorithms=(TargetAlgorithm(options.package_id),),
            ),
        ),
    )
    glove = sdk.glove()
    try:
        glove.connect()
        device = glove.device()
        pose = device.pose()
        output = device.output()
        pose.subscribe(print_pose)
        output.subscribe(print_output)

        glove.start()

        try:
            glove.run_forever()
        except KeyboardInterrupt:
            print("\nstopping")
    finally:
        glove.close()


if __name__ == "__main__":
    main()
