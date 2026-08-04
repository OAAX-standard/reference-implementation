"""
Stage 1 integration tests: YOLO model simplification and inference validation.

Uses the simplified_yolo_models session fixture (conftest.py) which simplifies
all variants once and caches them to tests/test_models/simplified/.

Run via Stage 1:
    python tests/stage1.py
Or directly:
    pytest tests/test_yolo_integration.py -v
"""

from pathlib import Path

import numpy as np
import onnxruntime as ort
import pytest

from tests.models import TEST_MODELS

YOLO_MODELS = ["yolov8n", "yolo11n", "yolo11s"]
YOLO_MODELS_B4 = ["yolo11n_b4", "yolo11s_b4"]
YOLO_MODELS_320 = ["yolo11n_320", "yolo11s_320"]
YOLO_MODELS_320_B4 = ["yolo11n_320_b4", "yolo11s_320_b4"]


def _infer(onnx_path: Path, model_name: str, img_value: float = 0.0) -> np.ndarray:
    info = TEST_MODELS[model_name]
    session = ort.InferenceSession(str(onnx_path))
    input_name = session.get_inputs()[0].name
    img = np.full(info["input_shape"], img_value, dtype=np.float32)
    return session.run(None, {input_name: img})[0]


def _get(fixture_dict: dict, model_name: str) -> Path:
    path = fixture_dict.get(model_name)
    if path is None:
        pytest.skip(f"{model_name} not available in fixture")
    return path


# ── File presence ─────────────────────────────────────────────────────────────


@pytest.mark.parametrize("model_name", YOLO_MODELS)
def test_simplified_file_exists(simplified_yolo_models, model_name):
    """Simplified .onnx file is present on disk."""
    assert _get(simplified_yolo_models, model_name).exists()


# ── ONNX Runtime loading ──────────────────────────────────────────────────────


@pytest.mark.parametrize("model_name", YOLO_MODELS)
def test_loads_in_onnxruntime(simplified_yolo_models, model_name):
    """Simplified model loads in ONNX Runtime with one input and one output."""
    path = _get(simplified_yolo_models, model_name)
    session = ort.InferenceSession(str(path))
    assert len(session.get_inputs()) == 1
    assert len(session.get_outputs()) == 1


# ── Output shape ──────────────────────────────────────────────────────────────


@pytest.mark.parametrize("model_name", YOLO_MODELS)
def test_output_shape(simplified_yolo_models, model_name):
    """Inference output matches expected YOLO shape [1, 84, 8400]."""
    path = _get(simplified_yolo_models, model_name)
    info = TEST_MODELS[model_name]
    output = _infer(path, model_name)
    assert output.shape == (
        1,
        info["output_channels"],
        info["output_anchors"],
    ), f"Expected (1, {info['output_channels']}, {info['output_anchors']}), got {output.shape}"


# ── Numerical sanity ──────────────────────────────────────────────────────────


@pytest.mark.parametrize("model_name", YOLO_MODELS)
def test_output_is_finite(simplified_yolo_models, model_name):
    """Inference output contains no NaN or Inf values."""
    path = _get(simplified_yolo_models, model_name)
    output = _infer(path, model_name)
    assert np.all(np.isfinite(output)), "Output has NaN/Inf"


@pytest.mark.parametrize("model_name", YOLO_MODELS)
def test_bbox_coords_in_range(simplified_yolo_models, model_name):
    """Bounding-box coordinates are within a reasonable range."""
    path = _get(simplified_yolo_models, model_name)
    info = TEST_MODELS[model_name]
    output = _infer(path, model_name, img_value=0.5)
    bbox = output[0, :4, :]
    assert np.all(np.isfinite(bbox))
    assert np.median(np.abs(bbox)) < info["input_shape"][2] * 10


# ── Batch=4 tests ─────────────────────────────────────────────────────────────


@pytest.mark.parametrize("model_name", YOLO_MODELS_B4)
def test_b4_file_exists(simplified_yolo_models_batch4, model_name):
    assert _get(simplified_yolo_models_batch4, model_name).exists()


@pytest.mark.parametrize("model_name", YOLO_MODELS_B4)
def test_b4_output_shape(simplified_yolo_models_batch4, model_name):
    """Batch=4 output matches expected shape [4, 84, 8400]."""
    path = _get(simplified_yolo_models_batch4, model_name)
    info = TEST_MODELS[model_name]
    output = _infer(path, model_name)
    assert output.shape == (
        4,
        info["output_channels"],
        info["output_anchors"],
    ), f"Expected (4, {info['output_channels']}, {info['output_anchors']}), got {output.shape}"


@pytest.mark.parametrize("model_name", YOLO_MODELS_B4)
def test_b4_output_is_finite(simplified_yolo_models_batch4, model_name):
    path = _get(simplified_yolo_models_batch4, model_name)
    output = _infer(path, model_name)
    assert np.all(np.isfinite(output))


# ── 320x320 tests ─────────────────────────────────────────────────────────────


@pytest.mark.parametrize("model_name", YOLO_MODELS_320)
def test_320_file_exists(simplified_yolo_models_320, model_name):
    assert _get(simplified_yolo_models_320, model_name).exists()


@pytest.mark.parametrize("model_name", YOLO_MODELS_320)
def test_320_output_shape(simplified_yolo_models_320, model_name):
    """320x320 output matches expected shape [1, 84, 2100]."""
    path = _get(simplified_yolo_models_320, model_name)
    info = TEST_MODELS[model_name]
    output = _infer(path, model_name)
    assert output.shape == (
        1,
        info["output_channels"],
        info["output_anchors"],
    )


@pytest.mark.parametrize("model_name", YOLO_MODELS_320)
def test_320_output_is_finite(simplified_yolo_models_320, model_name):
    path = _get(simplified_yolo_models_320, model_name)
    output = _infer(path, model_name)
    assert np.all(np.isfinite(output))


# ── 320x320 batch=4 tests ─────────────────────────────────────────────────────


@pytest.mark.parametrize("model_name", YOLO_MODELS_320_B4)
def test_320_b4_file_exists(simplified_yolo_models_320_batch4, model_name):
    assert _get(simplified_yolo_models_320_batch4, model_name).exists()


@pytest.mark.parametrize("model_name", YOLO_MODELS_320_B4)
def test_320_b4_output_shape(simplified_yolo_models_320_batch4, model_name):
    """320x320 batch=4 output matches expected shape [4, 84, 2100]."""
    path = _get(simplified_yolo_models_320_batch4, model_name)
    info = TEST_MODELS[model_name]
    output = _infer(path, model_name)
    assert output.shape == (
        4,
        info["output_channels"],
        info["output_anchors"],
    )


@pytest.mark.parametrize("model_name", YOLO_MODELS_320_B4)
def test_320_b4_output_is_finite(simplified_yolo_models_320_batch4, model_name):
    path = _get(simplified_yolo_models_320_batch4, model_name)
    output = _infer(path, model_name)
    assert np.all(np.isfinite(output))
