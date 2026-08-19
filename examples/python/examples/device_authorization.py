from __future__ import annotations

import argparse
import sys
from collections.abc import Callable
from pathlib import Path
from types import TracebackType

from qnbot_sdk import Sdk, SerialConnection, Side
from qnbot_sdk.glove import ActivationChallenge, GloveConfig


class _Cleanup:
    def __init__(self) -> None:
        self._primary: tuple[BaseException, TracebackType | None] | None = None
        self._cleanup_failure: RuntimeError | None = None

    def capture_primary(self, error: BaseException) -> None:
        if self._primary is None:
            self._primary = (error, error.__traceback__)

    def run(self, label: str, action: Callable[[], object]) -> bool:
        try:
            action()
            return True
        except BaseException as error:
            if self._cleanup_failure is None:
                self._cleanup_failure = RuntimeError(
                    f"qnbot example cleanup failed during {label} (details redacted)"
                )
            print(
                f"cleanup failed: {label} ({type(error).__name__})",
                file=sys.stderr,
            )
            return False

    def rethrow_if_failed(self) -> None:
        if self._primary is not None:
            error, traceback = self._primary
            raise error.with_traceback(traceback)
        if self._cleanup_failure is not None:
            raise self._cleanup_failure from None


def _cleanup_authorization_session(
    cleanup: _Cleanup,
    device: object,
    *,
    started: bool,
    factory_active: bool,
    connected: bool,
) -> tuple[bool, bool, bool]:
    if started and cleanup.run("device.stop()", device.stop):  # type: ignore[attr-defined]
        started = False
    if (
        not started
        and factory_active
        and cleanup.run(
            "device.exit_factory_calibration()",
            device.exit_factory_calibration,  # type: ignore[attr-defined]
        )
    ):
        factory_active = False
    if (
        not started
        and not factory_active
        and connected
        and cleanup.run(
            "device.disconnect()",
            device.disconnect,  # type: ignore[attr-defined]
        )
    ):
        connected = False
    return started, factory_active, connected


def create_sdk(port: str, side: Side) -> Sdk:
    return Sdk(
        devices=(
            GloveConfig(
                side=side,
                connection=SerialConnection(port=port),
            ),
        )
    )


def load_service_response(
    challenge: ActivationChallenge,
    operation: str,
    ticket_path: Path,
    signature_path: Path,
) -> tuple[bytes, bytes]:
    """Application integration point for service response files.

    Replace these file reads with the application's authenticated activation
    service client. Send the Challenge fields and operation to that service;
    Ticket and signature stay opaque to the SDK.
    """
    _ = challenge, operation
    return ticket_path.read_bytes(), signature_path.read_bytes()


def main() -> None:
    arguments = argparse.ArgumentParser(
        description="Authorize one serial glove with application-supplied service output"
    )
    arguments.add_argument("--port", required=True)
    arguments.add_argument(
        "--side",
        choices=(Side.LEFT.value, Side.RIGHT.value),
        required=True,
        help="Physical glove side",
    )
    arguments.add_argument(
        "--operation",
        choices=("permanent", "deactivate", "factory"),
        required=True,
    )
    arguments.add_argument("--ticket", type=Path, required=True)
    arguments.add_argument("--signature", type=Path, required=True)
    options = arguments.parse_args()

    sdk = create_sdk(options.port, Side(options.side))
    glove = sdk.glove()
    glove.connect()
    device = glove.device()
    connected = True
    started = False
    factory_active = False
    cleanup = _Cleanup()
    try:
        info = device.get_device_info()
        status = device.activation_status()
        print(
            f"device model={info.model} firmware={info.firmware_version} "
            f"activation={status.state.name}"
        )

        challenge = device.activation_challenge()
        ticket_bytes, signature = load_service_response(
            challenge,
            options.operation,
            options.ticket,
            options.signature,
        )
        if options.operation == "permanent":
            result = device.activate_permanently(ticket_bytes, signature)
            print(f"permanent activation={result.outcome.name}")
        elif options.operation == "deactivate":
            result = device.revoke_permanent_activation(ticket_bytes, signature)
            print(f"permanent deactivation={result.outcome.name}")
        else:
            factory = device.authorize_factory_calibration(ticket_bytes, signature)
            factory_active = factory.active
            print(f"factory authorization active={factory.active}")

        if options.operation != "deactivate":
            device.start()
            started = True
            print("telemetry started")
            device.stop()
            started = False
            print("telemetry stopped")

        if factory_active:
            device.exit_factory_calibration()
            factory_active = False
            print("factory authorization exited")

        device.disconnect()
        connected = False
    except BaseException as error:
        cleanup.capture_primary(error)

    started, factory_active, connected = _cleanup_authorization_session(
        cleanup,
        device,
        started=started,
        factory_active=factory_active,
        connected=connected,
    )
    if not connected:
        cleanup.run("glove.close()", glove.close)
    cleanup.rethrow_if_failed()


if __name__ == "__main__":
    main()
