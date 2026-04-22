#!/usr/bin/env python3
"""Stage 2: Benchmark the OAAX CPU runtime vs. ONNX Runtime (Python API).

Requires tests/test_models/simplified/ to be populated by stage1 first.
Runs yolo_test (OAAX) and onnxruntime.InferenceSession (ORT) for each model,
then prints a side-by-side comparison with a speedup ratio.

Usage:
    python tests/stage2.py [--runs 300] [--warmup 5] [--csv results.csv]
                           [--skip-runtime] [--skip-ort]
"""

import argparse
import csv
import os
import platform
import re
import shutil
import subprocess
import sys
import time
from datetime import UTC, datetime, timezone
from pathlib import Path

import numpy as np
import onnxruntime as ort

ROOT = Path(__file__).parent.parent
SIMPLIFIED_DIR = ROOT / "tests" / "test_models" / "simplified"
RUNTIME_BUILD_DIR = ROOT / "runtime-library" / "build"
TEST_BUILD_DIR = ROOT / "tests" / "runtime" / "build"
IS_WINDOWS = platform.system() == "Windows"

YOLO_MODELS = {
    "yolov8n",
    "yolo11n",
    "yolo11s",
    "yolo11n_b4",
    "yolo11s_b4",
    "yolo11n_320",
    "yolo11s_320",
    "yolo11n_320_b4",
    "yolo11s_320_b4",
}


def header(title: str) -> None:
    print(f"\n\033[34m=== {title} ===\033[0m")


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--runs", type=int, default=300, help="Inference runs per model (default: 300)")
    p.add_argument("--warmup", type=int, default=5, help="Warmup runs (default: 5)")
    p.add_argument("--csv", default="", help="Path to output CSV file")
    p.add_argument("--skip-runtime", action="store_true", help="Skip OAAX yolo_test section")
    p.add_argument("--skip-ort", action="store_true", help="Skip ORT Python baseline section")
    return p.parse_args()


# ── Binary paths ──────────────────────────────────────────────────────────────


def yolo_test_path() -> Path:
    if IS_WINDOWS:
        return TEST_BUILD_DIR / "Release" / "yolo_test.exe"
    return TEST_BUILD_DIR / "yolo_test"


def simple_test_path() -> Path:
    if IS_WINDOWS:
        return TEST_BUILD_DIR / "Release" / "simple_test.exe"
    return TEST_BUILD_DIR / "simple_test"


# ── Build yolo_test ────────────────────────────────────────────────────────────


def build_runtime_tests() -> bool:
    cmake = os.environ.get("CMAKE_BIN", shutil.which("cmake") or "cmake")
    runtime_lib_dir = str(RUNTIME_BUILD_DIR / "Release") if IS_WINDOWS else str(RUNTIME_BUILD_DIR)

    TEST_BUILD_DIR.mkdir(parents=True, exist_ok=True)
    try:
        subprocess.run(
            [cmake, "..", f"-DRUNTIME_LIB_DIR={runtime_lib_dir}", "-DCMAKE_BUILD_TYPE=Release"],
            cwd=TEST_BUILD_DIR,
            check=True,
        )
        subprocess.run(
            [cmake, "--build", ".", "--config", "Release", "-j", str(os.cpu_count() or 4)],
            cwd=TEST_BUILD_DIR,
            check=True,
        )
        if IS_WINDOWS:
            dst_dir = TEST_BUILD_DIR / "Release"
            dst_dir.mkdir(parents=True, exist_ok=True)
            for dll in (RUNTIME_BUILD_DIR / "Release").glob("*.dll"):
                dst = dst_dir / dll.name
                if not dst.exists():
                    shutil.copy2(dll, dst)
        else:
            for so in RUNTIME_BUILD_DIR.glob("*.so*"):
                dst = TEST_BUILD_DIR / so.name
                if not dst.exists():
                    dst.symlink_to(so)
        return True
    except subprocess.CalledProcessError as e:
        print(f"  Build failed: {e}")
        return False


# ── Model discovery ────────────────────────────────────────────────────────────


def get_simplified_models() -> list:
    return [
        (p, p.stem.replace("-simplified", ""))
        for p in sorted(SIMPLIFIED_DIR.glob("*-simplified.onnx"))
        if p.stem.replace("-simplified", "") in YOLO_MODELS
    ]


# ── OAAX yolo_test benchmark ──────────────────────────────────────────────────


def parse_field(text: str, pattern: str) -> str:
    m = re.search(pattern, text)
    return m.group(1) if m else ""


def run_process(cmd: list, cwd=None, env=None, timeout: int = 120) -> str | None:
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=cwd, env=env, timeout=timeout)
        if result.returncode != 0:
            print(f"  [exit {result.returncode}] {Path(cmd[0]).name}")
        return result.stdout + result.stderr
    except (subprocess.TimeoutExpired, FileNotFoundError) as e:
        print(f"  [error] {e}")
        return None


def run_simple_test() -> bool:
    binary = simple_test_path()
    if not binary.exists():
        print(f"  simple_test not found at {binary}")
        return False
    env = os.environ.copy()
    if not IS_WINDOWS:
        env["LD_LIBRARY_PATH"] = f"{TEST_BUILD_DIR}:{env.get('LD_LIBRARY_PATH', '')}"
    text = run_process([str(binary)], cwd=binary.parent, env=env, timeout=60)
    if text and "All tests passed" in text:
        print("  simple_test: PASS")
        return True
    print(f"  simple_test: FAIL\n{(text or '')[:400]}")
    return False


def run_yolo_test(onnx_path: Path, warmup: int, runs: int, batch: int = 1, imgsz: int = 640) -> tuple | None:
    binary = yolo_test_path()
    if not binary.exists():
        print(f"  [error] yolo_test binary not found: {binary}")
        return None
    env = os.environ.copy()
    if not IS_WINDOWS:
        env["LD_LIBRARY_PATH"] = f"{TEST_BUILD_DIR}:{env.get('LD_LIBRARY_PATH', '')}"
    cmd = [str(binary), str(onnx_path), "--warmup", str(warmup), "--runs", str(runs)]
    if batch > 1:
        cmd += ["--batch", str(batch)]
    if imgsz != 640:
        cmd += ["--imgsz", str(imgsz)]
    text = run_process(cmd, cwd=binary.parent, env=env, timeout=600 + runs * 2)
    if not text or "=== Results ===" not in text:
        if text:
            print(f"  [output] {text[:400]}")
        return None
    result = (
        parse_field(text, r"Avg latency:\s+([\d.]+)"),
        parse_field(text, r"Min latency:\s+([\d.]+)"),
        parse_field(text, r"p95 latency:\s+([\d.]+)"),
        parse_field(text, r"Throughput\s*:\s*([\d.]+)"),
    )
    if not any(result):
        print(f"  [output] {text[:400]}")
        return None
    return result


# ── ORT Python baseline benchmark ─────────────────────────────────────────────


def run_ort_benchmark(onnx_path: Path, warmup: int, runs: int, batch: int = 1, imgsz: int = 640) -> tuple | None:
    try:
        sess_opts = ort.SessionOptions()
        sess_opts.graph_optimization_level = ort.GraphOptimizationLevel.ORT_ENABLE_ALL
        sess = ort.InferenceSession(str(onnx_path), sess_options=sess_opts, providers=["CPUExecutionProvider"])

        inp_name = sess.get_inputs()[0].name
        data = np.random.rand(batch, 3, imgsz, imgsz).astype(np.float32)
        feed = {inp_name: data}

        for _ in range(warmup):
            sess.run(None, feed)

        latencies_ms = []
        for _ in range(runs):
            t0 = time.perf_counter()
            sess.run(None, feed)
            latencies_ms.append((time.perf_counter() - t0) * 1000.0)

        latencies_ms.sort()
        avg = sum(latencies_ms) / len(latencies_ms)
        min_l = latencies_ms[0]
        p95 = latencies_ms[int(len(latencies_ms) * 0.95)]
        fps = batch * 1000.0 / avg

        return (f"{avg:.3f}", f"{min_l:.3f}", f"{p95:.3f}", f"{fps:.4f}")
    except Exception as e:
        print(f"  [ORT error] {e}")
        return None


# ── Formatting ────────────────────────────────────────────────────────────────

_HDR = "  {:<20}  {:>9}  {:>9}  {:>10}  {:>9}  {:>9}  {:>10}  {:>8}"
_SEP = "  {:<20}  {:>9}  {:>9}  {:>10}  {:>9}  {:>9}  {:>10}  {:>8}"
_ROW = "  {:<20}  {:>8}ms  {:>8}ms  {:>8} FPS  {:>8}ms  {:>8}ms  {:>8} FPS  {:>7}x"


def print_table_header() -> None:
    print(_HDR.format("Model", "OAAX avg", "OAAX p95", "OAAX FPS", "ORT avg", "ORT p95", "ORT FPS", "speedup"))
    print(_SEP.format("-" * 20, "-" * 9, "-" * 9, "-" * 10, "-" * 9, "-" * 9, "-" * 10, "-" * 8))


def speedup_str(oaax_avg: str, ort_avg: str) -> str:
    try:
        ratio = float(ort_avg) / float(oaax_avg)
        return f"{ratio:.2f}"
    except (ValueError, ZeroDivisionError):
        return "n/a"


def write_csv_row(writer, model: str, oaax: tuple | None, ort_r: tuple | None) -> None:
    if not writer:
        return
    row = {
        "timestamp": datetime.now(UTC).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "model": model,
        "oaax_avg_ms": oaax[0] if oaax else "",
        "oaax_min_ms": oaax[1] if oaax else "",
        "oaax_p95_ms": oaax[2] if oaax else "",
        "oaax_fps": oaax[3] if oaax else "",
        "ort_avg_ms": ort_r[0] if ort_r else "",
        "ort_min_ms": ort_r[1] if ort_r else "",
        "ort_p95_ms": ort_r[2] if ort_r else "",
        "ort_fps": ort_r[3] if ort_r else "",
        "speedup": speedup_str(oaax[0], ort_r[0]) if (oaax and ort_r) else "",
    }
    writer.writerow(row)


# ── Main ──────────────────────────────────────────────────────────────────────


def main() -> None:
    args = parse_args()

    if not SIMPLIFIED_DIR.exists() or not any(SIMPLIFIED_DIR.glob("*-simplified.onnx")):
        print("ERROR: tests/test_models/simplified/ not found or empty — run stage1 first")
        sys.exit(1)

    models = get_simplified_models()
    if not models:
        print("ERROR: no recognized YOLO models found in simplified dir")
        sys.exit(1)

    csv_file = None
    csv_writer = None
    if args.csv:
        csv_file = open(args.csv, "w", newline="")
        csv_writer = csv.DictWriter(
            csv_file,
            fieldnames=[
                "timestamp",
                "model",
                "oaax_avg_ms",
                "oaax_min_ms",
                "oaax_p95_ms",
                "oaax_fps",
                "ort_avg_ms",
                "ort_min_ms",
                "ort_p95_ms",
                "ort_fps",
                "speedup",
            ],
        )
        csv_writer.writeheader()

    try:
        if not args.skip_runtime:
            if not yolo_test_path().exists():
                print(f"  yolo_test not found at {yolo_test_path()}, building...")
                if not build_runtime_tests():
                    print("  Build failed — aborting")
                    sys.exit(1)

            header("Step 1: simple_test (API health check)")
            run_simple_test()

        header(f"Step 2: OAAX vs ORT  (warmup={args.warmup}, runs={args.runs})")
        print(f"  ORT version: {ort.__version__}")
        print_table_header()

        pass_count = fail_count = 0
        for onnx_path, model_name in models:
            batch = 4 if model_name.endswith("_b4") else 1
            imgsz = 320 if "_320" in model_name else 640

            oaax_r = None if args.skip_runtime else run_yolo_test(onnx_path, args.warmup, args.runs, batch, imgsz)
            ort_r = None if args.skip_ort else run_ort_benchmark(onnx_path, args.warmup, args.runs, batch, imgsz)

            oaax_avg = oaax_r[0] if oaax_r else "n/a"
            oaax_p95 = oaax_r[2] if oaax_r else "n/a"
            oaax_fps = oaax_r[3] if oaax_r else "n/a"
            ort_avg = ort_r[0] if ort_r else "n/a"
            ort_p95 = ort_r[2] if ort_r else "n/a"
            ort_fps = ort_r[3] if ort_r else "n/a"
            spd = speedup_str(oaax_r[0], ort_r[0]) if (oaax_r and ort_r) else "n/a"

            print(_ROW.format(model_name, oaax_avg, oaax_p95, oaax_fps, ort_avg, ort_p95, ort_fps, spd))
            write_csv_row(csv_writer, model_name, oaax_r, ort_r)

            if oaax_r or ort_r:
                pass_count += 1
            else:
                fail_count += 1

        header("Stage 2 results")
        if args.csv:
            print(f"  CSV saved to: {args.csv}")
        print(f"  Models benchmarked: {pass_count}")
        print(f"  Models failed:      {fail_count}")
        print()
        print("  speedup > 1.0x means OAAX is faster than ORT")

        if fail_count > 0:
            sys.exit(1)

    finally:
        if csv_file:
            csv_file.close()


if __name__ == "__main__":
    main()
