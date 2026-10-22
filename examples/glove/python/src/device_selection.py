from __future__ import annotations

from qnbot_sdk import AmbiguousSelectionError, Sdk, Side
from qnbot_sdk.glove import ExternalConnection, GloveConfig


def main() -> None:
    sdk = Sdk(
        devices=(
            GloveConfig(
                name="primary", side=Side.LEFT, connection=ExternalConnection()
            ),
            GloveConfig(
                name="primary", side=Side.RIGHT, connection=ExternalConnection()
            ),
            GloveConfig(name="backup", side=Side.LEFT, connection=ExternalConnection()),
        ),
    )
    glove = sdk.glove()
    try:
        primary_left = glove.device(name="primary", side=Side.LEFT)
        primary_right = glove.device(name="primary", side=Side.RIGHT)
        backup_left = glove.device(name="backup")
        print(f"primary left source_id={primary_left.source_id}")
        print(f"primary right source_id={primary_right.source_id}")
        print(f"backup left source_id={backup_left.source_id}")

        try:
            glove.device(name="primary")
        except AmbiguousSelectionError:
            print("expected ambiguity: name=primary")
        else:
            raise RuntimeError("name=primary should match two devices")

        try:
            glove.device(side=Side.LEFT)
        except AmbiguousSelectionError:
            print("expected ambiguity: side=left")
        else:
            raise RuntimeError("side=left should match two devices")
    finally:
        glove.close()
    print("device selection complete")


if __name__ == "__main__":
    main()
