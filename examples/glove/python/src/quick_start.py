from __future__ import annotations

from qnbot_sdk import Sample, Sdk
from qnbot_sdk.glove import GloveConfig, GlovePose


def print_pose(sample: Sample[GlovePose]) -> None:
    print(
        f"pose sequence={sample.sequence} "
        f"fingertips={sample.value.payload.fingertip_local}"
    )


def main() -> None:
    sdk = Sdk(devices=(GloveConfig(),))
    glove = sdk.glove()
    device = glove.device()
    device.pose().subscribe(print_pose)

    try:
        glove.start()
        print("running; press Ctrl+C to stop")
        glove.run_forever()
    finally:
        glove.close()


if __name__ == "__main__":
    main()
