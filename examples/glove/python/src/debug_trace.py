from __future__ import annotations

import argparse
from qnbot_sdk import (
    DebugConfig,
    DebugDetail,
    DebugModule,
    DeviceSelector,
    Sample,
    Sdk,
    SerialConnection,
    Side,
    TargetAlgorithm,
    TargetConfig,
)
from qnbot_sdk.glove import GloveConfig, GlovePose, HandJointCommand

def print_pose(origin: str, sample: Sample[GlovePose]) -> None:
    print(f"{origin} pose sequence={sample.sequence}")


def print_output(sample: Sample[HandJointCommand]) -> None:
    print(
        f"retargeting sequence={sample.sequence} "
        f"target={sample.value.target} joints={sample.value.joints}"
    )


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Read glove poses with sampled debug tracing enabled"
    )
    arguments.add_argument("--port", required=True, help="Serial port for the glove")
    arguments.add_argument(
        "--side",
        choices=(Side.LEFT.value, Side.RIGHT.value),
        required=True,
        help="Physical glove side",
    )
    arguments.add_argument("--sample-rate", type=int, default=10)
    arguments.add_argument(
        "--detail",
        choices=tuple(detail.value for detail in DebugDetail),
        default=DebugDetail.SUMMARY.value,
    )
    arguments.add_argument("--package-id", required=True)
    options = arguments.parse_args()
    if options.sample_rate <= 0:
        arguments.error("--sample-rate must be greater than 0")

    side = Side(options.side)
    source = DeviceSelector(type="glove", side=side)
    modules = (
        DebugModule.TRANSPORT,
        DebugModule.DEVICE,
        DebugModule.CAPTURE,
        DebugModule.CALIBRATION,
        DebugModule.RETARGETING,
        DebugModule.OUTPUT,
    )
    debug = DebugConfig(
        modules=modules,
        detail=DebugDetail(options.detail),
        sample_rate=options.sample_rate,
    )
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
        debug=debug,
    )
    glove = sdk.glove()
    device = glove.device()

    try:
        device.pose().subscribe(lambda sample: print_pose("callback", sample))
        device.output(name="openxr_hand").subscribe(print_output)
        glove.start()
        print("ready; press Ctrl+C to stop")
        glove.run_forever()
    finally:
        glove.close()


if __name__ == "__main__":
    main()
