"""
Docker integration tests for the OAAX CPU conversion toolchain.

Verifies the Docker image works end-to-end: given a plain .onnx file it
produces a simplified .onnx and a logs.json.

Prerequisites:
  - Docker installed and running
  - Image built: bash conversion-toolchain/build-toolchain.sh
"""

import json
import shutil
import subprocess
import tempfile
import time
from pathlib import Path

import pytest

from tests.models import download_model

DOCKER_IMAGE = "oaax-cpu-toolchain:latest"


def _docker_available() -> bool:
    try:
        r = subprocess.run(["docker", "info"], capture_output=True, timeout=5)
        if r.returncode != 0:
            return False
        r = subprocess.run(["docker", "images", "-q", DOCKER_IMAGE], capture_output=True, text=True, timeout=5)
        return bool(r.stdout.strip())
    except Exception:
        return False


@pytest.fixture(scope="session")
def docker_check():
    if not _docker_available():
        pytest.skip(
            f"Docker not available or image '{DOCKER_IMAGE}' not built. "
            f"Build with: bash conversion-toolchain/build-toolchain.sh"
        )


@pytest.fixture
def workspace():
    d = tempfile.mkdtemp()
    inp = Path(d) / "input"
    out = Path(d) / "output"
    inp.mkdir()
    out.mkdir()
    yield {"input": inp, "output": out}
    shutil.rmtree(d)


@pytest.fixture
def sample_model():
    models_dir = Path(__file__).parent / "test_models"
    models_dir.mkdir(exist_ok=True)
    model_path = download_model("squeezenet", str(models_dir))
    if not Path(model_path).exists():
        pytest.skip("Sample model not available")
    return model_path


def _run_docker(inp: Path, out: Path, model_filename: str, timeout: int = 120) -> subprocess.CompletedProcess:
    return subprocess.run(
        [
            "docker",
            "run",
            "--rm",
            "-v",
            f"{inp}:/input",
            "-v",
            f"{out}:/output",
            DOCKER_IMAGE,
            f"/input/{model_filename}",
            "/output",
        ],
        capture_output=True,
        text=True,
        timeout=timeout,
    )


class TestDockerBasics:
    def test_image_exists(self, docker_check):
        r = subprocess.run(["docker", "images", "-q", DOCKER_IMAGE], capture_output=True, text=True)
        assert r.stdout.strip(), f"Docker image {DOCKER_IMAGE} not found"

    def test_help_command(self, docker_check):
        r = subprocess.run(
            ["docker", "run", "--rm", DOCKER_IMAGE, "--help"], capture_output=True, text=True, timeout=30
        )
        assert r.returncode in (0, 1)  # argparse exits 0 or 1 on --help


class TestDockerConversion:
    def test_simple_conversion(self, docker_check, workspace, sample_model):
        """Basic conversion produces simplified .onnx and logs.json."""
        shutil.copy2(sample_model, workspace["input"] / Path(sample_model).name)

        r = _run_docker(workspace["input"], workspace["output"], Path(sample_model).name)
        assert r.returncode == 0, f"Conversion failed: {r.stderr}"

        output_onnx = list(workspace["output"].glob("*-simplified.onnx"))
        assert len(output_onnx) == 1, "Expected one simplified .onnx"

        logs_file = workspace["output"] / "logs.json"
        assert logs_file.exists(), "No logs.json produced"
        logs = json.loads(logs_file.read_text())
        assert isinstance(logs, list) and len(logs) > 0
        messages = [m.get("Message", "") for m in logs]
        assert any("Successful" in msg for msg in messages), f"No success message: {messages}"

    def test_output_loads_in_onnxruntime(self, docker_check, workspace, sample_model):
        """Simplified output is a valid ONNX model."""
        import onnxruntime as ort

        shutil.copy2(sample_model, workspace["input"] / Path(sample_model).name)
        r = _run_docker(workspace["input"], workspace["output"], Path(sample_model).name)
        assert r.returncode == 0

        output_onnx = list(workspace["output"].glob("*-simplified.onnx"))[0]
        session = ort.InferenceSession(str(output_onnx))
        assert len(session.get_inputs()) > 0
        assert len(session.get_outputs()) > 0


class TestDockerErrorHandling:
    def test_nonexistent_input(self, docker_check, workspace):
        r = _run_docker(workspace["input"], workspace["output"], "nonexistent.onnx", timeout=30)
        assert r.returncode != 0

    def test_corrupted_onnx(self, docker_check, workspace):
        bad = workspace["input"] / "bad.onnx"
        bad.write_text("not a valid onnx file")
        r = _run_docker(workspace["input"], workspace["output"], "bad.onnx", timeout=30)
        assert r.returncode != 0


class TestDockerPerformance:
    def test_conversion_time(self, docker_check, workspace, sample_model):
        """Conversion completes within 60 seconds for a small model."""
        shutil.copy2(sample_model, workspace["input"] / Path(sample_model).name)
        start = time.time()
        r = _run_docker(workspace["input"], workspace["output"], Path(sample_model).name)
        elapsed = time.time() - start
        assert r.returncode == 0, "Conversion failed"
        assert elapsed < 60, f"Conversion took {elapsed:.1f}s (> 60s limit)"


if __name__ == "__main__":
    pytest.main([__file__, "-v", "--tb=short"])
