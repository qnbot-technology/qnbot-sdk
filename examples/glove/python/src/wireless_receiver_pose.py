from __future__ import annotations

from qnbot_sdk import Sample, Sdk, Side
from qnbot_sdk.glove import GloveConfig, GlovePose, GloveReceiverConfig


def print_pose(side: str, sample: Sample[GlovePose]) -> None:
    print(
        f"{side} pose sequence={sample.sequence} "
        f"fingertips={sample.value.payload.fingertip_local}"
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
            device.pose().subscribe(
                lambda sample, side=side: print_pose(side.value, sample)
            )
        glove.start()
        print("running; press Ctrl+C to stop")
        glove.run_forever()
    finally:
        glove.close()


if __name__ == "__main__":
    main()
