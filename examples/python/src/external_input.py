from __future__ import annotations

import threading

from qnbot_sdk import (
    DeviceSelector,
    Sample,
    Sdk,
    Side,
    TargetConfig,
    TargetType,
)
from qnbot_sdk.glove import (
    ExternalConnection,
    ExternalGloveFrame,
    GloveConfig,
    GloveNodePose,
    GlovePose,
    HandJointCommand,
)


def node(x: float, y: float, z: float) -> GloveNodePose:
    return GloveNodePose(
        position=(x, y, z),
        quaternion_xyzw=(0.0, 0.0, 0.0, 1.0),
    )


def frame(step: int) -> ExternalGloveFrame:
    offset = step * 0.001
    return ExternalGloveFrame(
        thumb=node(0.01 + offset, 0.02, 0.03),
        index=node(0.02 + offset, 0.03, 0.04),
        middle=node(0.03 + offset, 0.04, 0.05),
        ring=node(0.04 + offset, 0.05, 0.06),
        pinky=node(0.05 + offset, 0.06, 0.07),
        timestamp_sec=step * 0.01,
    )


def print_pose(sample: Sample[GlovePose]) -> None:
    print(f"pose callback sequence={sample.sequence}")


def main() -> None:
    sdk = Sdk(
        devices=[
            GloveConfig(side=Side.RIGHT, connection=ExternalConnection()),
        ],
        targets=[
            TargetConfig(
                type=TargetType.HAND,
                name="hand",
                side=Side.RIGHT,
                source=DeviceSelector(type="glove", side=Side.RIGHT),
            )
        ],
    )
    glove = sdk.glove()
    glove.connect()
    device = glove.device()
    pose = device.pose()
    output = device.output(name="hand")
    output_ready = threading.Event()

    def print_output(sample: Sample[HandJointCommand]) -> None:
        print(f"pass-through output callback sequence={sample.sequence}")
        output_ready.set()

    pose.subscribe(print_pose)
    output.subscribe(print_output)
    try:
        device.start()
        glove.run_background()
        try:
            for step in range(1, 4):
                current = device.push_frame(frame(step))
                print(f"pushed pose sequence={current.meta.sequence}")

            if not output_ready.wait(timeout=1.0):
                raise RuntimeError("external input did not publish pass-through output")
            latest_pose = pose.latest()
            latest_output = output.latest()
            if latest_pose is None or latest_output is None:
                raise RuntimeError(
                    "external input did not publish pose and pass-through output"
                )
            print(f"latest pose sequence={latest_pose.sequence}")
            print(f"latest pass-through output sequence={latest_output.sequence}")
            print("external input complete")
        finally:
            glove.request_stop()
            glove.join()
    finally:
        glove.close()


if __name__ == "__main__":
    main()
