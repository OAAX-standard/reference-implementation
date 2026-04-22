"""
Download and export ONNX models for testing.

Classification models are downloaded from the ONNX Model Zoo.
YOLO models are exported via ultralytics.
"""

import shutil
import urllib.request
from pathlib import Path

# Maps YOLO model name → (ultralytics .pt name, export batch size, imgsz)
_YOLO_EXPORTS = {
    "yolov8n": ("yolov8n.pt", 1, 640),
    "yolo11n": ("yolo11n.pt", 1, 640),
    "yolo11s": ("yolo11s.pt", 1, 640),
    "yolo11n_b4": ("yolo11n.pt", 4, 640),
    "yolo11s_b4": ("yolo11s.pt", 4, 640),
    "yolo11n_320": ("yolo11n.pt", 1, 320),
    "yolo11s_320": ("yolo11s.pt", 1, 320),
    "yolo11n_320_b4": ("yolo11n.pt", 4, 320),
    "yolo11s_320_b4": ("yolo11s.pt", 4, 320),
}

TEST_MODELS = {
    "resnet18": {
        "url": "https://github.com/onnx/models/raw/main/validated/vision/classification/resnet/model/resnet18-v1-7.onnx",
        "filename": "resnet18-v1-7.onnx",
        "task": "image_classification",
        "input_shape": [1, 3, 224, 224],
    },
    "mobilenetv2": {
        "url": "https://github.com/onnx/models/raw/main/validated/vision/classification/mobilenet/model/mobilenetv2-7.onnx",
        "filename": "mobilenetv2-7.onnx",
        "task": "image_classification",
        "input_shape": [1, 3, 224, 224],
    },
    "squeezenet": {
        "url": "https://github.com/onnx/models/raw/main/validated/vision/classification/squeezenet/model/squeezenet1.0-7.onnx",
        "filename": "squeezenet1.0-7.onnx",
        "task": "image_classification",
        "input_shape": [1, 3, 224, 224],
    },
    "yolov8n": {
        "filename": "yolov8n.onnx",
        "task": "object_detection",
        "input_shape": [1, 3, 640, 640],
        "input_name": "images",
        "output_channels": 84,
        "output_anchors": 8400,
    },
    "yolo11n": {
        "filename": "yolo11n.onnx",
        "task": "object_detection",
        "input_shape": [1, 3, 640, 640],
        "input_name": "images",
        "output_channels": 84,
        "output_anchors": 8400,
    },
    "yolo11s": {
        "filename": "yolo11s.onnx",
        "task": "object_detection",
        "input_shape": [1, 3, 640, 640],
        "input_name": "images",
        "output_channels": 84,
        "output_anchors": 8400,
    },
    "yolo11n_b4": {
        "filename": "yolo11n_b4.onnx",
        "task": "object_detection",
        "input_shape": [4, 3, 640, 640],
        "input_name": "images",
        "output_channels": 84,
        "output_anchors": 8400,
    },
    "yolo11s_b4": {
        "filename": "yolo11s_b4.onnx",
        "task": "object_detection",
        "input_shape": [4, 3, 640, 640],
        "input_name": "images",
        "output_channels": 84,
        "output_anchors": 8400,
    },
    "yolo11n_320": {
        "filename": "yolo11n_320.onnx",
        "task": "object_detection",
        "input_shape": [1, 3, 320, 320],
        "input_name": "images",
        "output_channels": 84,
        "output_anchors": 2100,
    },
    "yolo11s_320": {
        "filename": "yolo11s_320.onnx",
        "task": "object_detection",
        "input_shape": [1, 3, 320, 320],
        "input_name": "images",
        "output_channels": 84,
        "output_anchors": 2100,
    },
    "yolo11n_320_b4": {
        "filename": "yolo11n_320_b4.onnx",
        "task": "object_detection",
        "input_shape": [4, 3, 320, 320],
        "input_name": "images",
        "output_channels": 84,
        "output_anchors": 2100,
    },
    "yolo11s_320_b4": {
        "filename": "yolo11s_320_b4.onnx",
        "task": "object_detection",
        "input_shape": [4, 3, 320, 320],
        "input_name": "images",
        "output_channels": 84,
        "output_anchors": 2100,
    },
}


def export_yolo_model(model_name: str, output_dir: str) -> str:
    """Export a YOLO model to ONNX using ultralytics."""
    if model_name not in _YOLO_EXPORTS:
        raise ValueError(f"Unknown YOLO export model: {model_name}")

    try:
        from ultralytics import YOLO
    except ImportError as e:
        raise ImportError("ultralytics is required for YOLO models. Install with: pip install ultralytics") from e

    model_info = TEST_MODELS[model_name]
    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)
    dest = output_path / model_info["filename"]

    if dest.exists():
        print(f"Model already exists: {dest}")
        return str(dest)

    ultralytics_name, batch, imgsz = _YOLO_EXPORTS[model_name]
    print(f"Exporting {model_name} to ONNX (batch={batch}, imgsz={imgsz})...")
    model = YOLO(ultralytics_name)
    export_kwargs = dict(format="onnx", imgsz=imgsz, opset=12, simplify=False)
    if batch > 1:
        export_kwargs["batch"] = batch
    exported = model.export(**export_kwargs)
    shutil.move(str(exported), str(dest))
    print(f"Exported to: {dest}")
    return str(dest)


def download_model(model_name: str, output_dir: str = "test_models") -> str:
    """Download or export a test model. Returns path to the model file."""
    if model_name not in TEST_MODELS:
        raise ValueError(f"Unknown model: {model_name}. Available: {list(TEST_MODELS.keys())}")

    model_info = TEST_MODELS[model_name]
    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)
    model_path = output_path / model_info["filename"]

    if model_path.exists():
        print(f"Model already exists: {model_path}")
        return str(model_path)

    if model_name in _YOLO_EXPORTS:
        return export_yolo_model(model_name, output_dir)

    print(f"Downloading {model_name} from ONNX Model Zoo...")
    try:
        urllib.request.urlretrieve(model_info["url"], str(model_path))
        print(f"Downloaded: {model_path}")
        return str(model_path)
    except Exception as e:
        print(f"Failed to download {model_name}: {e}")
        raise


def download_all_models(output_dir: str = "test_models") -> None:
    """Download/export all test models."""
    for model_name in TEST_MODELS:
        try:
            download_model(model_name, output_dir)
        except Exception as e:
            print(f"Warning: failed to prepare {model_name}: {e}")


def get_model_info(model_name: str) -> dict:
    """Return metadata for a test model."""
    if model_name not in TEST_MODELS:
        raise ValueError(f"Unknown model: {model_name}")
    return TEST_MODELS[model_name].copy()


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Download ONNX test models")
    parser.add_argument("--model", choices=list(TEST_MODELS.keys()) + ["all"], default="all")
    parser.add_argument("--output-dir", default="tests/test_models")
    args = parser.parse_args()

    if args.model == "all":
        download_all_models(args.output_dir)
    else:
        download_model(args.model, args.output_dir)
