from __future__ import annotations

from qnbot_sdk import CompositeExoGloveConfig, Sdk, Side
from qnbot_sdk.exo import ExoConfig
from qnbot_sdk.glove import GloveConfig


def main() -> None:
    sdk = Sdk(
        devices=(
            CompositeExoGloveConfig(
                devices=(
                    GloveConfig(side=Side.LEFT),
                    GloveConfig(side=Side.RIGHT),
                    ExoConfig(),
                ),
            ),
        )
    )
    try:
        glove = sdk.glove()
        exo = sdk.exo()
        left_glove_device = glove.device(side=Side.LEFT)
        right_glove_device = glove.device(side=Side.RIGHT)
        exo_device = exo.device()
        left_glove_device.pose().subscribe(
            lambda sample: print(
                f"left glove pose sequence={sample.sequence} "
                f"fingertips={len(sample.value.payload.fingertip_local)}"
            )
        )
        right_glove_device.pose().subscribe(
            lambda sample: print(
                f"right glove pose sequence={sample.sequence} "
                f"fingertips={len(sample.value.payload.fingertip_local)}"
            )
        )
        sdk.start()
        print(
            f"started Left Glove={left_glove_device.source_id} "
            f"Right Glove={right_glove_device.source_id} "
            f"Exo={exo_device.source_id}",
        )
        print("press Ctrl+C to stop")
        # There are no work targets here, so the taskless runtime waits
        # until Ctrl+C requests a stop. The root SDK lifecycle drives both
        # member domains through one SDK instance.
        sdk.run_forever()
    except KeyboardInterrupt:
        sdk.stop()
    finally:
        sdk.close()


if __name__ == "__main__":
    main()
