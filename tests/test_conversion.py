"""
Unit tests for the OAAX conversion toolchain (onnxsim simplification).

Tests with real ONNX models: ResNet-18, MobileNetV2, SqueezeNet.
"""

import shutil
import tempfile
from pathlib import Path

import numpy as np
import onnxruntime as ort
import pytest
from google.protobuf.message import DecodeError

from conversion_toolchain.logger import Logs
from conversion_toolchain.utils import md5_hash, simplify_onnx
from tests.models import TEST_MODELS, download_model, get_model_info


class TestSimplification:
    """Test onnxsim simplification with real classification models."""

    @pytest.fixture(scope="class")
    def test_models_dir(self):
        models_dir = Path(__file__).parent / "test_models"
        models_dir.mkdir(exist_ok=True)
        for model_name in ("resnet18", "mobilenetv2", "squeezenet"):
            try:
                download_model(model_name, str(models_dir))
            except Exception as e:
                print(f"Warning: could not download {model_name}: {e}")
        return str(models_dir)

    @pytest.fixture
    def tmp(self):
        d = tempfile.mkdtemp()
        yield d
        shutil.rmtree(d)

    @pytest.fixture(params=["resnet18", "mobilenetv2", "squeezenet"])
    def real_model(self, request, test_models_dir):
        model_name = request.param
        model_info = get_model_info(model_name)
        model_path = Path(test_models_dir) / model_info["filename"]
        if not model_path.exists():
            pytest.skip(f"Model {model_name} not available")
        return {"name": model_name, "path": str(model_path), "info": model_info}

    def test_simplification_produces_onnx(self, real_model, tmp):
        """simplify_onnx writes a valid .onnx file."""
        logs = Logs()
        out = simplify_onnx(real_model["path"], logs)
        assert Path(out).exists(), f"Simplified file not found: {out}"
        assert out.endswith(".onnx"), "Output should be an .onnx file"

    def test_simplified_loads_in_onnxruntime(self, real_model, tmp):
        """Simplified model can be loaded by onnxruntime."""
        logs = Logs()
        out = simplify_onnx(real_model["path"], logs)
        session = ort.InferenceSession(out)
        assert len(session.get_inputs()) > 0
        assert len(session.get_outputs()) > 0

    def test_simplified_runs_inference(self, real_model, tmp):
        """Simplified model runs inference and produces float32 output."""
        logs = Logs()
        out = simplify_onnx(real_model["path"], logs)
        session = ort.InferenceSession(out)
        input_shape = real_model["info"]["input_shape"]
        dummy = np.random.randn(*input_shape).astype(np.float32)
        input_name = session.get_inputs()[0].name
        result = session.run(None, {input_name: dummy})
        assert result and result[0].dtype == np.float32
        assert result[0].size > 0

    def test_simplification_logged(self, real_model, tmp):
        """Logs capture the simplification step."""
        logs = Logs()
        simplify_onnx(real_model["path"], logs)
        messages = [m.message for m in logs.messages]
        assert any("simplif" in m.lower() for m in messages)


class TestUtilities:
    """Tests for utility functions."""

    def test_md5_hash_consistency(self):
        import tempfile

        with tempfile.NamedTemporaryFile(mode="w", delete=False, suffix=".txt") as f:
            f.write("test content")
            temp_path = f.name

        try:
            h1 = md5_hash(temp_path)
            h2 = md5_hash(temp_path)
            assert h1 == h2
            assert len(h1) == 32
        finally:
            Path(temp_path).unlink()

    def test_logger_add_message(self):
        logs = Logs()
        logs.add_message("hello", {"k": "v"})
        assert len(logs.messages) == 1
        assert logs.messages[0].message == "hello"
        assert logs.messages[0].data == {"k": "v"}

    def test_logger_add_data(self):
        logs = Logs()
        logs.add_message("base")
        logs.add_data(extra="value")
        assert logs.messages[0].data["extra"] == "value"

    def test_logger_json_serialization(self):
        import json

        logs = Logs()
        logs.add_message("msg", {"key": "val"})
        parsed = json.loads(str(logs))
        assert isinstance(parsed, list)
        assert parsed[0]["Message"] == "msg"


class TestErrorHandling:
    """Test error handling for invalid inputs."""

    @pytest.fixture
    def tmp(self):
        d = tempfile.mkdtemp()
        yield d
        shutil.rmtree(d)

    def test_nonexistent_path(self, tmp):
        """simplify_onnx raises an error for a missing file."""
        logs = Logs()
        with pytest.raises((FileNotFoundError, OSError, RuntimeError, ValueError)):
            simplify_onnx("/tmp/nonexistent_model_xyz.onnx", logs)

    def test_corrupted_onnx(self, tmp):
        """simplify_onnx raises an error for a corrupt file."""
        bad = Path(tmp) / "bad.onnx"
        bad.write_text("not a valid onnx model")
        logs = Logs()
        with pytest.raises((OSError, RuntimeError, ValueError, DecodeError)):
            simplify_onnx(str(bad), logs)


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
