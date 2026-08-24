from __future__ import annotations

import argparse
import asyncio

from qnbot_sdk import (
    DeviceSelector,
    Sample,
    Sdk,
    SerialConnection,
    Side,
    TargetAlgorithm,
    TargetConfig,
)
from qnbot_sdk.glove import GloveConfig, HandJointCommand

DEFAULT_PACKAGE_ID = "qnbot-dexhand"
DEFAULT_TARGET_NAME = "openxr_hand"


def create_sdk(port: str, side: Side, target_name: str, package_id: str) -> Sdk:
    source = DeviceSelector(type="glove", name="primary")
    return Sdk(
        devices=(
            GloveConfig(
                name="primary",
                side=side,
                connection=SerialConnection(port=port),
            ),
        ),
        targets=(
            TargetConfig(
                name=target_name,
                side=side,
                source=source,
                algorithms=(TargetAlgorithm(id=package_id),),
            ),
        ),
    )


def print_output(origin: str, sample: Sample[HandJointCommand]) -> None:
    print(
        f"{origin} retargeting sequence={sample.sequence} "
        f"target={sample.value.target} joints={sample.value.joints}"
    )


async def run(options: argparse.Namespace) -> None:
    sdk = create_sdk(
        options.port,
        Side(options.side),
        options.target_name,
        options.package_id,
    )
    glove = sdk.glove()
    try:
        glove.connect()
        output = glove.device().output(name=options.target_name)
        glove.start()

        glove.run_background()
        try:
            print_output("next", await output.next())

            received = 0
            async for sample in output:
                print_output("stream", sample)
                received += 1
                if received >= options.samples:
                    break
        finally:
            glove.request_stop()
            await asyncio.to_thread(glove.join)
        print("async stopped")
    finally:
        await asyncio.to_thread(glove.close)


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Consume retargeted hand output with Python asyncio"
    )
    arguments.add_argument("--port", required=True, help="Serial port for the glove")
    arguments.add_argument(
        "--side",
        choices=(Side.LEFT.value, Side.RIGHT.value),
        required=True,
        help="Physical glove side",
    )
    arguments.add_argument("--samples", type=int, default=10)
    arguments.add_argument("--target-name", default=DEFAULT_TARGET_NAME)
    arguments.add_argument("--package-id", default=DEFAULT_PACKAGE_ID)
    options = arguments.parse_args()
    if options.samples < 1:
        arguments.error("--samples must be at least 1")
    asyncio.run(run(options))


if __name__ == "__main__":
    main()
