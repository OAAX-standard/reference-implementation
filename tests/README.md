# Test Suite Overview

This document describes all tests in the OAAX reference implementation.

---

## Python Tests (pytest)

Run with: `uv run pytest tests/`

### `test_conversion.py` — Conversion toolchain unit tests

**`TestSimplification`**
- `test_models_dir` — verifies the test models directory exists and contains at least one `.onnx` file
- `test_simplification_produces_onnx` — runs `simplify_onnx` on a real model and checks the output file is created
- `test_simplified_loads_in_onnxruntime` — confirms the simplified model loads without error in ORT
- `test_simplified_runs_inference` — runs one forward pass on the simplified model and checks the output shape
- `test_simplification_logged` — checks that the JSON log written by `simplify_onnx` contains the expected step entry

**`TestUtilities`**
- `test_md5_hash_consistency` — same file produces the same MD5 hash on two calls
- `test_logger_add_message` — `ConversionLogger.add_message` stores entries with the correct fields
- `test_logger_add_data` — `ConversionLogger.add_data` stores arbitrary metadata
- `test_logger_json_serialization` — the logger produces valid JSON when written to disk

**`TestErrorHandling`**
- `test_nonexistent_path` — `simplify_onnx` raises an appropriate exception for a missing file
- `test_corrupted_onnx` — `simplify_onnx` raises when given a file that is not valid ONNX

### `test_docker.py` — Conversion toolchain Docker integration tests

Requires the `oaax-cpu-toolchain` Docker image to be present (skip otherwise).

**`TestDockerBasics`**
- `test_image_exists` — the Docker image is present on the host
- `test_help_command` — the container starts and prints help without error

**`TestDockerConversion`**
- `test_simple_conversion` — mounts a sample ONNX model, runs the container, checks the log contains a success message
- `test_output_loads_in_onnxruntime` — the file produced by the container loads in ORT

**`TestDockerErrorHandling`**
- `test_nonexistent_input` — container exits with a non-zero code when the input path does not exist
- `test_corrupted_onnx` — container exits with a non-zero code when the ONNX file is corrupted

**`TestDockerPerformance`**
- `test_conversion_time` — conversion of the sample model completes within a 60-second wall-clock budget

### `test_yolo_integration.py` — YOLO model output validation

Requires `tests/test_models/simplified/` to be populated (run `stage1.py` first).
Parameterised over `yolov8n`, `yolo11n`, `yolo11s` at 640 px.

- `test_simplified_file_exists` — simplified model file is present on disk
- `test_loads_in_onnxruntime` — simplified model loads in ORT without error
- `test_output_shape` — output tensor shape matches expected YOLO layout `(1, 84, 8400)`
- `test_output_is_finite` — no NaN or Inf values in the output
- `test_bbox_coords_in_range` — bounding-box coordinates are in `[0, 1]`

Batch-4 variants (`test_b4_*`) repeat the above for batch size 4 (`yolo11n_b4`, `yolo11s_b4`).
320-px variants (`test_320_*`) repeat for 320-pixel input (`yolo11n_320`, `yolo11s_320`).
320-px batch-4 variants (`test_320_b4_*`) cover `yolo11n_320_b4`, `yolo11s_320_b4`.

---

## Integration Scripts

### `stage1.py` — End-to-end conversion pipeline

Runs the full conversion suite: downloads YOLO models via Ultralytics, converts each to a simplified ONNX, then runs the pytest suites above. Populates `tests/test_models/simplified/` for use by `stage2.py`.

```bash
uv run python tests/stage1.py
```

### `stage2.py` — Runtime benchmark vs. ORT baseline

Builds and runs the C++ `yolo_test` binary against each simplified model, then runs the same models through ORT's Python API and prints a side-by-side latency/throughput table with speedup ratios. Optionally writes results to a CSV file.

```bash
uv run python tests/stage2.py --runs 300 --csv results.csv
```

Flags: `--runs N`, `--warmup N`, `--csv <path>`, `--skip-runtime`, `--skip-ort`

---

## C++ Tests

Build with: `bash tests/runtime/build-tests.sh`
Run from `tests/runtime/build/` with `LD_LIBRARY_PATH=.`

### `simple_test` — C API smoke tests (no model required)

Exercises the full init/cleanup lifecycle and error paths without needing an ONNX model:

1. `runtime_init()` succeeds
2. `runtime_get_version()` / `runtime_get_name()` return expected values
3. Double `runtime_init()` returns `ALREADY_INITIALIZED`
4. `runtime_enqueue_input()` returns `MODEL_NOT_LOADED` before any model is loaded
5. `runtime_load_models()` fails gracefully for a non-existent path
6. `runtime_get_error()` is set after a failure
7. `runtime_cleanup()` succeeds and is idempotent

With an `.onnx` path as argument, also tests:

8. Model loading succeeds
9. `runtime_retrieve_output` on an empty queue returns `NO_OUTPUT_AVAILABLE`
10. `runtime_get_info()` returns a valid JSON object

### `yolo_test` — Async inference benchmark

Runs a configurable number of inference passes through the async queue using dedicated producer/consumer threads. Reports avg/min/p95 latency and throughput. Used internally by `stage2.py`.

```
./yolo_test <model.onnx> [--runs N] [--warmup N] [--batch N] [--imgsz N] [--in-flight N]
```

### `multi_model_test` — Concurrent multi-model inference

Loads two models simultaneously and uses concurrent producer threads to enqueue inputs to both. Verifies:

- All outputs are received with no drops
- `model_id` on each output matches the producing model
- `Tensors.id` on each output echoes the value set on the input (request correlation)
- Output shapes are valid (`1 × 84 × 8400` for YOLO)

```
./multi_model_test <model.onnx> [model2.onnx]
```
