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
    pose = glove.device().pose()
    pose.subscribe(print_pose)

    glove.start()
    print("running; press Ctrl+C to stop", flush=True)
    glove.run_forever()


if __name__ == "__main__":
    main()
