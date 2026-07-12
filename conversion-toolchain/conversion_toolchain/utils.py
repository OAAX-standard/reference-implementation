import hashlib
from os.path import splitext
from time import perf_counter

from onnx import load, save
from onnxsim import simplify


def simplify_onnx(onnx_path: str, logs):
    model = load(onnx_path)
    nodes_before = len(model.graph.node)

    start = perf_counter()
    try:
        simplified, check = simplify(model, check_n=3)
        assert check, "Failed to simplify ONNX model"
        logs.add_message(
            "Simplified ONNX model successfully",
            {
                "Nodes before": nodes_before,
                "Nodes after": len(simplified.graph.node),
                "Duration (s)": round(perf_counter() - start, 2),
            },
        )
        model = simplified
    except Exception as e:
        if logs:
            logs.add_message(
                "Failed to simplify ONNX model", {"Error": str(e), "Fallback": "Using the original ONNX file"}
            )

    simp_onnx_path = splitext(onnx_path)[0] + "-simplified.onnx"
    save(model, simp_onnx_path)

    return simp_onnx_path


def md5_hash(file_path: str) -> str:
    h = hashlib.md5()
    with open(file_path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()
