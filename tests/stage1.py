#!/usr/bin/env python3
"""Stage 1: Run conversion tests and simplify YOLO models via the Docker toolchain.

Output: tests/test_models/simplified/<model>-simplified.onnx
"""

import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).parent.parent
sys.path.insert(0, str(ROOT))

from tests.docker_utils import DOCKER_IMAGE, NO_CACHE, check_image, docker_user_args  # noqa: E402
from tests.models import download_model  # noqa: E402

SIMPLIFIED_DIR = ROOT / "tests" / "test_models" / "simplified"
CLASSIFICATION_MODELS = ["resnet18", "mobilenetv2", "squeezenet"]


def header(title: str) -> None:
    print(f"\n\033[34m=== {title} ===\033[0m")


def simplify_classification_models() -> None:
    """Download classification models and simplify them via the Docker toolchain."""
    image_problem = check_image()
    if image_problem:
        print(f"  Skipping: {image_problem}")
        return

    SIMPLIFIED_DIR.mkdir(parents=True, exist_ok=True)
    onnx_dir = ROOT / "tests" / "test_models" / "onnx"
    onnx_dir.mkdir(parents=True, exist_ok=True)

    for model_name in CLASSIFICATION_MODELS:
        dest = SIMPLIFIED_DIR / f"{model_name}-simplified.onnx"
        if dest.exists() and not NO_CACHE:
            print(f"  {model_name}: already simplified")
            continue

        print(f"  Downloading {model_name}...")
        onnx_path = Path(download_model(model_name, str(onnx_dir)))

        print(f"  Simplifying {model_name} via Docker toolchain...")
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            shutil.copy2(onnx_path, tmp_path / onnx_path.name)
            docker_out = tmp_path / "output"
            docker_out.mkdir()

            result = subprocess.run(
                [
                    "docker",
                    "run",
                    "--rm",
                    *docker_user_args(),
                    "-v",
                    f"{tmp_path}:/input",
                    "-v",
                    f"{docker_out}:/output",
                    DOCKER_IMAGE,
                    f"/input/{onnx_path.name}",
                    "/output",
                ],
                capture_output=True,
                text=True,
                timeout=300,
            )

            if result.returncode != 0:
                print(f"  FAIL: Docker simplification failed for {model_name}: {result.stderr[:200]}")
                continue

            outputs = list(docker_out.glob("*-simplified.onnx"))
            if not outputs:
                print(f"  FAIL: no simplified .onnx produced for {model_name}")
                continue

            shutil.copy2(outputs[0], dest)
            print(f"  {model_name}: simplified → {dest.name}")


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

    header("Step 6: Classification model simplification")
    simplify_classification_models()

    print("\nStage 1 complete — simplified models saved to tests/test_models/simplified/")


if __name__ == "__main__":
    main()
