from __future__ import annotations

import argparse
import time

from qnbot_sdk import Sdk, SerialConnection, Side
from qnbot_sdk.glove import (
    CalibrationMode,
    CaptureQualitySession,
    GloveConfig,
    GloveFinger,
    GloveHandPose,
)

STAGES = (
    "close",
    "open",
    "mcp90",
    "mcp90_pip90",
    "pinch_first",
    "pinch_middle",
    "pinch_ring",
    "pinch_little",
)
PROMPTS = {
    "close": "五指自然伸直并拢，放松保持，不握拳",
    "open": "五指自然伸直张开，保持姿势，不要过度用力",
    "mcp90": "四指 MCP 弯约 90 度，其余关节伸直，拇指放松并保持",
    "mcp90_pip90": "四指 MCP 和 PIP 向掌心弯曲，DIP 尽量伸直，拇指放松并保持",
    "pinch_first": "拇指与食指指腹相对轻触，保持姿势，不要挤压",
    "pinch_middle": "拇指与中指指腹相对轻触，保持姿势，不要挤压",
    "pinch_ring": "拇指与无名指指腹相对轻触，保持姿势，不要挤压",
    "pinch_little": "拇指与小指指腹相对轻触，保持姿势，不要挤压",
}


def collect_window(
    device,
    glove,
    detector: CaptureQualitySession,
) -> list[GloveHandPose]:
    """The application collects one new window from the existing pose channel."""
    pose = device.pose()
    previous = pose.latest()
    sequence = previous.sequence if previous else None
    window: list[GloveHandPose] = []
    count = detector.required_sample_count()
    while len(window) < count:
        glove.update()
        sample = pose.latest()
        if sample is None or sample.sequence == sequence:
            time.sleep(0.005)
            continue
        sequence = sample.sequence
        hand = sample.value.payload.hand
        if hand is None or not hand.fresh:
            continue
        if any(
            not hand.raw_valid_mask[finger] or hand.held_mask[finger]
            for finger in GloveFinger
        ):
            window.clear()
            continue
        window.append(hand)
    return window


def main() -> None:
    parser = argparse.ArgumentParser(
        description="独立检查当前手势，不启动 Capture 或 Calibration"
    )
    parser.add_argument("--port", required=True)
    parser.add_argument("--side", choices=("left", "right"), required=True)
    parser.add_argument("--stage", choices=STAGES, default="open")
    options = parser.parse_args()
    side = Side(options.side)
    sdk = Sdk(
        devices=(
            GloveConfig(side=side, connection=SerialConnection(port=options.port)),
        )
    )
    glove = sdk.glove()
    try:
        device = glove.device()
        with device.create_capture_quality_session() as detector:
            device.start(CalibrationMode.NONE)
            for stage in dict.fromkeys(("close", options.stage)):
                input(f"{stage}: {PROMPTS[stage]}。姿势稳定后按 Enter 收集一窗：")
                window = collect_window(device, glove, detector)
                result = detector.assess_stage(stage, window)
                print(
                    f"stage={stage} accepted={result.accepted} "
                    f"action={result.action.value} "
                    f"reason={result.reason} reason_en={result.reason_en}"
                )
                if not result.accepted:
                    break
    finally:
        glove.close()


if __name__ == "__main__":
    main()
