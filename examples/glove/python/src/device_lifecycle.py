from __future__ import annotations

import argparse
import time

from qnbot_sdk import Sample, Sdk, SerialConnection, Side, SleepResult
from qnbot_sdk.glove import GloveConfig, GlovePose


def print_pose(name: str, sample: Sample[GlovePose]) -> None:
    print(
        f"{name} pose sequence={sample.sequence} "
        f"fingertips={sample.value.payload.fingertip_local}"
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Stop and restart one glove while the other keeps streaming"
    )
    parser.add_argument("--left-port", help="Optional left glove or Receiver port")
    parser.add_argument("--right-port", help="Optional right glove or Receiver port")
    parser.add_argument(
        "--updates", type=int, default=10, help="Fresh samples per phase"
    )
    options = parser.parse_args()
    if options.updates < 1:
        parser.error("--updates must be at least 1")

    sdk = Sdk(
        devices=(
            GloveConfig(
                name="left",
                side=Side.LEFT,
                connection=SerialConnection(port=options.left_port)
                if options.left_port
                else SerialConnection(),
            ),
            GloveConfig(
                name="right",
                side=Side.RIGHT,
                connection=SerialConnection(port=options.right_port)
                if options.right_port
                else SerialConnection(),
            ),
        ),
    )
    glove = sdk.glove()
    try:
        left = glove.device(name="left")
        right = glove.device(name="right")
        left_pose = left.pose()
        right_pose = right.pose()

        def observe(*, right_active: bool) -> None:
            left_previous = left_pose.latest()
            right_previous = right_pose.latest()
            left_sequence = left_previous.sequence if left_previous else None
            right_sequence = right_previous.sequence if right_previous else None
            left_count = right_count = 0
            deadline = time.monotonic() + 10.0
            while left_count < options.updates or (
                right_active and right_count < options.updates
            ):
                if time.monotonic() >= deadline:
                    raise RuntimeError("timed out waiting for pose samples")
                update = glove.update()
                left_sample = left_pose.latest()
                right_sample = right_pose.latest()
                if (
                    not right_active
                    and (right_sample.sequence if right_sample else None)
                    != right_sequence
                ):
                    raise RuntimeError("right pose advanced while stopped")
                if left_sample is not None and left_sample.sequence != left_sequence:
                    left_sequence = left_sample.sequence
                    left_count += 1
                    print_pose("left", left_sample)
                if (
                    right_active
                    and right_sample is not None
                    and right_sample.sequence != right_sequence
                ):
                    right_sequence = right_sample.sequence
                    right_count += 1
                    print_pose("right", right_sample)
                if update.sleep() is SleepResult.NO_TASKS:
                    time.sleep(0.01)

        left.start()
        right.start()
        print("both devices active")
        observe(right_active=True)

        right.stop()
        print("right device stopped; left remains active")
        observe(right_active=False)

        right.start()
        print("right device restarted")
        observe(right_active=True)
        print("lifecycle complete")
    finally:
        glove.close()


if __name__ == "__main__":
    main()
