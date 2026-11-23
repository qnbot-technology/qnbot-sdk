from __future__ import annotations

import argparse
import asyncio

from qnbot_sdk import (
    ClosedHandleError,
    DeviceSelector,
    LifecycleError,
    Sample,
    Sdk,
    SerialConnection,
    Side,
    TargetAlgorithm,
    TargetConfig,
)
from qnbot_sdk.glove import GloveConfig, HandJointCommand


def print_output(origin: str, sample: Sample[HandJointCommand]) -> None:
    print(
        f"{origin} retargeting sequence={sample.sequence} "
        f"target={sample.value.target} joints={sample.value.joints}"
    )


async def run(options: argparse.Namespace) -> None:
    side = Side(options.side)
    source = DeviceSelector(type="glove", side=side)
    sdk = Sdk(
        devices=(
            GloveConfig(
                side=side,
                connection=SerialConnection(port=options.port),
            ),
        ),
        targets=(
            TargetConfig(
                name="openxr_hand",
                side=side,
                source=source,
                algorithms=(TargetAlgorithm(id=options.package_id),),
            ),
        ),
    )
    glove = sdk.glove()
    runner: asyncio.Task[None] | None = None
    consumer: asyncio.Task[None] | None = None

    async def consume() -> None:
        device = glove.device()
        output = device.output(name="openxr_hand")
        print_output("next", await output.next())
        received = 0
        async for sample in output:
            print_output("stream", sample)
            received += 1
            if received >= options.samples:
                break

    failure: BaseException | None = None
    try:
        glove.start()
        glove.run_background()
        runner = asyncio.create_task(asyncio.to_thread(glove.join))
        consumer = asyncio.create_task(consume())
        done, _ = await asyncio.wait(
            (runner, consumer), return_when=asyncio.FIRST_COMPLETED
        )
        await (runner if runner in done else consumer)
    except BaseException as error:  # noqa: BLE001 - cleanup still has to run.
        failure = error
    finally:
        if consumer is not None:
            consumer.cancel()
        if runner is not None:
            if not runner.done():
                try:
                    glove.request_stop()
                except (LifecycleError, ClosedHandleError):
                    # The runner can finish before the stop request arrives.
                    pass
            await asyncio.gather(runner, return_exceptions=True)
        if consumer is not None:
            await asyncio.gather(consumer, return_exceptions=True)
        await asyncio.to_thread(glove.close)
        await asyncio.to_thread(sdk.close)
    if failure is not None:
        raise failure
    print("async stopped")


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
    arguments.add_argument("--package-id", required=True)
    options = arguments.parse_args()
    if options.samples < 1:
        arguments.error("--samples must be at least 1")
    asyncio.run(run(options))


if __name__ == "__main__":
    main()
