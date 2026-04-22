#!/usr/bin/env python3
"""Stage 1: Run conversion tests and simplify YOLO models via the Docker toolchain.

Output: tests/test_models/simplified/<model>-simplified.onnx
"""

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).parent.parent


def header(title: str) -> None:
    print(f"\n\033[34m=== {title} ===\033[0m")


def main() -> None:
    header("Step 1: Conversion unit tests")
    subprocess.run(
        [sys.executable, "-m", "pytest", "tests/test_conversion.py", "-v", "--tb=short"],
        cwd=ROOT,
        check=True,
    )

    header("Step 2: YOLO 640x640 batch=1 integration tests")
    subprocess.run(
        [
            sys.executable,
            "-m",
            "pytest",
            "tests/test_yolo_integration.py",
            "-v",
            "--tb=short",
            "-k",
            "not b4 and not 320",
        ],
        cwd=ROOT,
        check=True,
    )

    header("Step 3: YOLO 640x640 batch=4 integration tests")
    subprocess.run(
        [sys.executable, "-m", "pytest", "tests/test_yolo_integration.py", "-v", "--tb=short", "-k", "b4 and not 320"],
        cwd=ROOT,
        check=True,
    )

    header("Step 4: YOLO 320x320 batch=1 integration tests")
    subprocess.run(
        [sys.executable, "-m", "pytest", "tests/test_yolo_integration.py", "-v", "--tb=short", "-k", "320 and not b4"],
        cwd=ROOT,
        check=True,
    )

    header("Step 5: YOLO 320x320 batch=4 integration tests")
    subprocess.run(
        [sys.executable, "-m", "pytest", "tests/test_yolo_integration.py", "-v", "--tb=short", "-k", "320 and b4"],
        cwd=ROOT,
        check=True,
    )

    print("\nStage 1 complete — simplified models saved to tests/test_models/simplified/")


if __name__ == "__main__":
    main()
