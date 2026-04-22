"""
Shared session fixtures for the OAAX reference-implementation test suite.

simplified_yolo_models runs all YOLO variants through the Docker toolchain
(onnxsim) once per session, caching results to tests/test_models/simplified/.
Stage 1 populates this cache; Stage 2 reads from it without re-converting.
"""

import subprocess
import tempfile
from pathlib import Path

import pytest

from tests.models import download_model

SIMPLIFIED_DIR = Path(__file__).parent / "test_models" / "simplified"
DOCKER_IMAGE = "oaax-cpu-toolchain:latest"

YOLO_MODELS = ["yolov8n", "yolo11n", "yolo11s"]
YOLO_MODELS_B4 = ["yolo11n_b4", "yolo11s_b4"]
YOLO_MODELS_320 = ["yolo11n_320", "yolo11s_320"]
YOLO_MODELS_320_B4 = ["yolo11n_320_b4", "yolo11s_320_b4"]


def _docker_image_available() -> bool:
    try:
        r = subprocess.run(["docker", "info"], capture_output=True, timeout=5)
        if r.returncode != 0:
            return False
        r = subprocess.run(["docker", "images", "-q", DOCKER_IMAGE], capture_output=True, text=True, timeout=5)
        return bool(r.stdout.strip())
    except Exception:
        return False


def _simplify_with_docker(model_name: str, onnx_path: Path, out_dir: Path) -> Path:
    """Simplify one model via the Docker toolchain image. Returns path to output .onnx."""
    out_dir.mkdir(parents=True, exist_ok=True)
    dest = out_dir / f"{model_name}-simplified.onnx"
    if dest.exists():
        return dest

    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp)
        import shutil

        shutil.copy2(onnx_path, tmp_path / onnx_path.name)
        docker_out = tmp_path / "output"
        docker_out.mkdir()

        result = subprocess.run(
            [
                "docker",
                "run",
                "--rm",
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
            raise RuntimeError(
                f"Docker simplification failed for {model_name} "
                f"(exit {result.returncode}):\n{result.stdout}\n{result.stderr}"
            )

        output_onnx = list(docker_out.glob("*-simplified.onnx"))
        if not output_onnx:
            raise RuntimeError(f"No simplified .onnx produced for {model_name}")

        shutil.copy2(output_onnx[0], dest)

    return dest


def _build_fixture(model_list: list) -> dict:
    """Download and simplify each model in model_list. Returns {model_name: Path}."""
    if not _docker_image_available():
        pytest.skip(
            f"Docker image '{DOCKER_IMAGE}' not available. Build with: bash conversion-toolchain/build-toolchain.sh"
        )

    try:
        import ultralytics  # noqa: F401
    except ImportError:
        pytest.skip("ultralytics not installed — run: uv sync --extra integration")

    onnx_dir = Path(__file__).parent / "test_models" / "onnx"
    onnx_dir.mkdir(parents=True, exist_ok=True)

    result = {}
    for model_name in model_list:
        onnx = Path(download_model(model_name, str(onnx_dir)))
        simplified = _simplify_with_docker(model_name, onnx, SIMPLIFIED_DIR)
        result[model_name] = simplified

    return result


@pytest.fixture(scope="session")
def simplified_yolo_models() -> dict:
    """Simplify YOLO 640x640 batch=1 models. Returns {model_name: Path-to-simplified-onnx}."""
    return _build_fixture(YOLO_MODELS)


@pytest.fixture(scope="session")
def simplified_yolo_models_batch4() -> dict:
    """Simplify YOLO 640x640 batch=4 models. Returns {model_name: Path-to-simplified-onnx}."""
    return _build_fixture(YOLO_MODELS_B4)


@pytest.fixture(scope="session")
def simplified_yolo_models_320() -> dict:
    """Simplify YOLO 320x320 batch=1 models. Returns {model_name: Path-to-simplified-onnx}."""
    return _build_fixture(YOLO_MODELS_320)


@pytest.fixture(scope="session")
def simplified_yolo_models_320_batch4() -> dict:
    """Simplify YOLO 320x320 batch=4 models. Returns {model_name: Path-to-simplified-onnx}."""
    return _build_fixture(YOLO_MODELS_320_B4)
