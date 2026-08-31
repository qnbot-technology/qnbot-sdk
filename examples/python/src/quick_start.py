from __future__ import annotations

import sys

from qnbot_sdk import Sample, Sdk
from qnbot_sdk.glove import GloveConfig, GlovePose


def print_pose(sample: Sample[GlovePose]) -> None:
    print(
        f"pose sequence={sample.sequence} "
        f"fingertips={sample.value.payload.fingertip_local}"
    )


def main() -> None:
    if len(sys.argv) != 1:
        raise SystemExit("quick_start does not accept arguments")

    sdk = Sdk(devices=(GloveConfig(),))
    glove = sdk.glove()
    try:
        glove.connect()
        device = glove.device()
        pose = device.pose()
        pose.subscribe(print_pose)

        glove.start()

        try:
            glove.run_forever()
        except KeyboardInterrupt:
            print("\nstopping")
    finally:
        glove.close()


if __name__ == "__main__":
    main()
