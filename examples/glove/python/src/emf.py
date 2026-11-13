from __future__ import annotations

import argparse

from qnbot_sdk import Sample, Sdk, SerialConnection, Side, SubscriptionConfig
from qnbot_sdk.glove import GloveConfig, GloveEmf


def print_emf(sample: Sample[GloveEmf]) -> None:
    payload = sample.value.payload
    for channel in payload.channels:
        print(
            f"emf session={payload.session} frame_sequence={payload.frame_sequence} sync={payload.sync} channel={channel.channel_id} units={payload.units} li={channel.li} lq={channel.lq} hi={channel.hi} hq={channel.hq} invalid_mask={channel.invalid_mask}"
        )


def main() -> None:
    arguments = argparse.ArgumentParser(description="Read raw glove EMF observations")
    arguments.add_argument(
        "--port", required=True, help="Serial port for the glove or wireless Receiver"
    )
    arguments.add_argument(
        "--side",
        choices=(Side.LEFT.value, Side.RIGHT.value),
        required=True,
        help="Physical glove side",
    )
    options = arguments.parse_args()

    sdk = Sdk(
        devices=(
            GloveConfig(
                side=Side(options.side),
                connection=SerialConnection(port=options.port),
            ),
        )
    )
    glove = sdk.glove()
    device = glove.device()

    try:
        device.emf().with_config(SubscriptionConfig.fifo(512)).subscribe(print_emf)
        glove.start()
        print("running; press Ctrl+C to stop")
        glove.run_forever()
    finally:
        glove.close()


if __name__ == "__main__":
    main()
