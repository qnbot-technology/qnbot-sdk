from __future__ import annotations

import time

from qnbot_sdk import Sample, Sdk
from qnbot_sdk.glove import GloveConfig, GlovePose

SYNC_INTERVAL_SECONDS = 1.0


def print_pose(sample: Sample[GlovePose]) -> None:
    print(
        f"pose sequence={sample.sequence} "
        f"fingertips={sample.value.payload.fingertip_local}"
    )


def external_reference_time_ms() -> float:
    """Return an absolute Unix timestamp in milliseconds.

    This standalone example uses the host clock as a runnable placeholder.
    Replace it with the millisecond timestamp supplied by the application's
    external time source. The value passed to ``sync_time`` is milliseconds,
    not nanoseconds.
    """

    return time.time() * 1_000.0


def main() -> None:
    sdk = Sdk(devices=(GloveConfig(),))
    glove = sdk.glove()
    reference_time_ms = external_reference_time_ms()
    glove.sync_time(reference_time_ms)

    device = glove.device()
    device.pose().subscribe(print_pose)

    glove.start()
    glove.run_background()
    print(
        f"Glove time synchronized to {reference_time_ms:.3f} ms; "
        f"resynchronizing every {SYNC_INTERVAL_SECONDS:g} s; press Ctrl+C to stop"
    )
    try:
        while True:
            time.sleep(SYNC_INTERVAL_SECONDS)
            reference_time_ms = external_reference_time_ms()
            glove.sync_time(reference_time_ms)
            print(f"Glove time resynchronized to {reference_time_ms:.3f} ms")
    except KeyboardInterrupt:
        pass
    finally:
        glove.request_stop()
        glove.join()
        glove.close()


if __name__ == "__main__":
    main()
