from __future__ import annotations

from qnbot_sdk import Sample, Sdk
from qnbot_sdk.glove import GloveConfig, HandJointCommand


def print_skeleton(sample: Sample[HandJointCommand]) -> None:
    print(
        f"skeleton sequence={sample.sequence} "
        f"target={sample.value.target} joints={sample.value.joints}"
    )


def main() -> None:
    sdk = Sdk(devices=(GloveConfig(),))
    glove = sdk.glove()
    try:
        glove.connect()
        device = glove.device()
        skeleton = device.skeleton()
        subscription = skeleton.subscribe(print_skeleton)
        glove.start()

        try:
            glove.run_forever()
        except KeyboardInterrupt:
            print("\nstopping")
        finally:
            del subscription
    finally:
        glove.close()


if __name__ == "__main__":
    main()
