from __future__ import annotations

from qnbot_sdk import Sample, Sdk, Side
from qnbot_sdk.glove import GloveConfig, GloveReceiverConfig, HandJointCommand


def print_joint_angles(side: str, sample: Sample[HandJointCommand]) -> None:
    print(
        f"{side} skeleton sequence={sample.sequence} "
        f"target={sample.value.target} joints={sample.value.joints}"
    )


def main() -> None:
    sdk = Sdk(
        devices=(
            GloveReceiverConfig(
                devices=(
                    GloveConfig(side=Side.LEFT),
                    GloveConfig(side=Side.RIGHT),
                ),
            ),
        ),
    )
    glove = sdk.glove()
    try:
        for side in (Side.LEFT, Side.RIGHT):
            device = glove.device(side=side)
            skeleton = device.skeleton()
            skeleton.joint_angles().subscribe(
                lambda sample, side=side: print_joint_angles(side.value, sample)
            )
        glove.start()
        print("running; press Ctrl+C to stop")
        glove.run_forever()
    finally:
        glove.close()


if __name__ == "__main__":
    main()
