from __future__ import annotations

import argparse
import math
import time

from qnbot_sdk import (
    AlgorithmsConfig,
    CaptureConfig,
    DeviceSelector,
    Sample,
    Sdk,
    SerialConnection,
    Side,
    TargetAlgorithm,
    TargetConfig,
)
from qnbot_sdk.glove import GloveConfig, HandJointCommand


def print_output(sample: Sample[HandJointCommand]) -> None:
    print(
        f"retargeting sequence={sample.sequence} "
        f"target={sample.value.target} joints={sample.value.joints}"
    )


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Use bone configuration and algorithm parameter overrides"
    )
    arguments.add_argument("--port", required=True)
    arguments.add_argument("--side", choices=("left", "right"), required=True)
    arguments.add_argument("--package-id", required=True)
    arguments.add_argument("--operator-id", default="default")
    arguments.add_argument(
        "--use-bone-config", choices=("auto", "true", "false"), default="auto"
    )
    arguments.add_argument("--algorithm-config-path")
    arguments.add_argument("--seconds", type=float, default=10.0)
    options = arguments.parse_args()
    if not math.isfinite(options.seconds) or options.seconds <= 0:
        arguments.error("--seconds must be finite and greater than 0")
    side = Side(options.side)
    usage = {"auto": "auto", "true": True, "false": False}[options.use_bone_config]
    sdk = Sdk(
        devices=(
            GloveConfig(side=side, connection=SerialConnection(port=options.port)),
        ),
        algorithms=AlgorithmsConfig(
            capture=CaptureConfig(operator_id=options.operator_id)
        ),
        targets=(
            TargetConfig(
                name="configured_hand",
                side=side,
                source=DeviceSelector(type="glove", side=side),
                algorithms=(
                    TargetAlgorithm(
                        id=options.package_id,
                        use_bone_config=usage,
                        algorithm_config_path=options.algorithm_config_path,
                    ),
                ),
            ),
        ),
    )
    glove = sdk.glove()
    device = glove.device()
    try:
        device.output(name="configured_hand").subscribe(print_output)
        glove.start()
        glove.run_background()
        print(
            f"package={options.package_id} use_bone_config={options.use_bone_config} "
            f"algorithm_config_path={options.algorithm_config_path}",
            flush=True,
        )
        time.sleep(options.seconds)
        glove.request_stop()
        glove.join()
    finally:
        glove.close()
    print("algorithm configuration complete")


if __name__ == "__main__":
    main()
