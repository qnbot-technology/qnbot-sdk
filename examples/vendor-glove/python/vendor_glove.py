from __future__ import annotations

import argparse
from itertools import pairwise
from threading import Condition
from time import monotonic

from qnbot_sdk import Sample, Sdk, SerialConnection, Side, VendorGloveConfig


def main() -> None:
    parser = argparse.ArgumentParser(description="查询厂商手套并接收原始传感器块")
    parser.add_argument("--port", required=True)
    parser.add_argument("--side", required=True, choices=("left", "right"))
    parser.add_argument("--baudrate", type=int, default=115200)
    args = parser.parse_args()
    sdk = Sdk(
        devices=[
            VendorGloveConfig(
                connection=SerialConnection(port=args.port, baudrate=args.baudrate),
                side=Side(args.side),
                name="vendor-glove",
            )
        ]
    )
    subscription = None
    wake = Condition()
    sequences: list[int] = []

    def receive(sample: Sample[dict]) -> None:
        value = sample.value
        blocks = {bit: len(data) for bit, data in value["blocks"].items()}
        with wake:
            sequences.append(sample.sequence)
            print(
                f"sample={sample.sequence} wire_sequence={value['wire_sequence']} "
                f"bitmap={value['bitmap']} timestamp_ms={value['timestamp_ms']} blocks={blocks}"
            )
            wake.notify_all()

    try:
        sources = sdk.vendor_glove_sources()
        if len(sources) != 1:
            raise RuntimeError(f"需要一个厂商源，实际为 {len(sources)}")
        source = sources[0]
        sdk.start()
        for operation in ("read_info", "read_parameters"):
            reply = sdk.vendor_glove_command(
                source,
                {
                    "operation": operation,
                    "ids": [2, 6] if operation == "read_info" else [2, 3],
                },
            )
            values = {item["id"]: item["value"] for item in reply["values"]}
            print(f"{operation} status={reply['status']} values={values}")
        subscription = sdk.vendor_glove_raw_sensors(source).subscribe(receive)
        deadline = monotonic() + 10
        with wake:
            while len(sequences) < 10:
                remaining = deadline - monotonic()
                if remaining <= 0:
                    raise TimeoutError(
                        "10 秒内未收到 10 帧；检查设备模式、订阅表和连接"
                    )
                wake.wait(remaining)
            if any(a >= b for a, b in pairwise(sequences)):
                raise RuntimeError("SDK 样本序号未前进")
        sdk.stop()
    finally:
        try:
            if subscription is not None:
                subscription.unsubscribe()
        finally:
            sdk.close()


if __name__ == "__main__":
    main()
