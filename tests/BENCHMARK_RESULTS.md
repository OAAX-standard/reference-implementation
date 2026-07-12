# Benchmark Results

Tracks inference performance across runtime versions. **vs baseline** = `current / 4919f1c7` throughput ratio; values > 1.0x are improvements.

> ⚠️ **Results below were measured at commit `00c7eff3`, before the replica-based
> threading redesign (`4d6b6f96`) replaced inter-op threads** — the
> `intra=N inter=M` configurations in the tables no longer exist (inter-op is
> now fixed at 1; throughput scales via `num_replicas`). Re-run the procedure
> below on current HEAD to refresh. `stage2.py --csv` now stamps each row with
> the commit it measured.

## How to update

Run all models sequentially in isolation (no other CPU-intensive processes), 300 runs each:

```bash
cd tests/runtime/build
PERF=eco  # or power

for spec in "yolo11n images 640 1" "yolo11n_320 images 320 1" "yolo11s images 640 1" \
            "yolo11s_320 images 320 1" "yolov8n images 640 1" \
            "yolo11n_b4 images 640 4" "yolo11n_320_b4 images 320 4" \
            "yolo11s_b4 images 640 4" "yolo11s_320_b4 images 320 4"; do
  name=$(echo $spec | awk '{print $1}')
  iname=$(echo $spec | awk '{print $2}')
  sz=$(echo $spec | awk '{print $3}')
  batch=$(echo $spec | awk '{print $4}')
  echo "=== $name ==="
  LD_LIBRARY_PATH=. ./yolo_test ../../test_models/simplified/${name}-simplified.onnx \
    --warmup 5 --runs 300 --input-name $iname --imgsz $sz --batch $batch \
    --perf-mode $PERF 2>/dev/null | grep -E "Avg|p95|Throughput"
done

for spec in "mobilenetv2 data 224" "resnet18 data 224" "squeezenet data_0 224"; do
  name=$(echo $spec | awk '{print $1}')
  iname=$(echo $spec | awk '{print $2}')
  sz=$(echo $spec | awk '{print $3}')
  echo "=== $name ==="
  LD_LIBRARY_PATH=. ./yolo_test ../../test_models/simplified/${name}-simplified.onnx \
    --warmup 5 --runs 300 --input-name $iname --imgsz $sz \
    --no-validate --perf-mode $PERF 2>/dev/null | grep -E "Avg|p95|Throughput"
done
```

---

## Machine

| Field | Value |
|---|---|
| Platform | Linux X86_64 |
| Logical CPU cores | 20 |
| Warmup runs | 5 |
| In-flight requests | 5 |

---

## YOLO — batch 1

| Model | Input | Date | Commit | `perf_mode` | Runs | Avg (ms) | p95 (ms) | Throughput (img/s) | vs baseline |
|---|---|---|---|---|---:|---:|---:|---:|---:|
| yolo11n | 640×640 | 2026-06-08 | 4919f1c7 | n/a (4 threads) | 100 | 125.5 | 136.4 | 38.88 | baseline |
| yolo11n | 640×640 | 2026-06-08 | 00c7eff3 | eco (intra=5 inter=2) | 300 | 106.3 | 110.4 | 46.45 | **+1.20x** |
| yolo11n | 640×640 | 2026-06-08 | 00c7eff3 | power (intra=12 inter=6) | 300 | 105.1 | 107.2 | 47.08 | **+1.21x** |
| yolo11n_320 | 320×320 | 2026-06-08 | 4919f1c7 | n/a (4 threads) | 100 | 33.2 | 35.6 | 146.6 | baseline |
| yolo11n_320 | 320×320 | 2026-06-08 | 00c7eff3 | eco (intra=5 inter=2) | 300 | 28.4 | 30.6 | 173.6 | **+1.18x** |
| yolo11n_320 | 320×320 | 2026-06-08 | 00c7eff3 | power (intra=12 inter=6) | 300 | 27.9 | 29.0 | 176.6 | **+1.20x** |
| yolo11s | 640×640 | 2026-06-08 | 4919f1c7 | n/a (4 threads) | 100 | 343.5 | 387.4 | 14.25 | baseline |
| yolo11s | 640×640 | 2026-06-08 | 00c7eff3 | eco (intra=5 inter=2) | 300 | 268.3 | 278.3 | 18.46 | **+1.30x** |
| yolo11s | 640×640 | 2026-06-08 | 00c7eff3 | power (intra=12 inter=6) | 300 | 224.3 | 231.1 | 22.10 | **+1.55x** |
| yolo11s_320 | 320×320 | 2026-06-08 | 4919f1c7 | n/a (4 threads) | 100 | 79.0 | 85.1 | 61.81 | baseline |
| yolo11s_320 | 320×320 | 2026-06-08 | 00c7eff3 | eco (intra=5 inter=2) | 300 | 71.8 | 75.1 | 68.97 | **+1.12x** |
| yolo11s_320 | 320×320 | 2026-06-08 | 00c7eff3 | power (intra=12 inter=6) | 300 | 67.1 | 69.0 | 73.77 | **+1.19x** |
| yolov8n | 640×640 | 2026-06-08 | 00c7eff3 | eco (intra=5 inter=2) | 300 | 110.0 | 113.1 | 44.93 | baseline |
| yolov8n | 640×640 | 2026-06-08 | 00c7eff3 | power (intra=12 inter=6) | 300 | 113.8 | 117.4 | 43.50 | 0.97x |

## YOLO — batch 4

| Model | Input | Date | Commit | `perf_mode` | Runs | Avg (ms) | p95 (ms) | Throughput (img/s) | vs baseline |
|---|---|---|---|---|---:|---:|---:|---:|---:|
| yolo11n_b4 | 640×640 | 2026-06-08 | 4919f1c7 | n/a (4 threads) | 100 | 622.6 | 694.4 | 31.34 | baseline |
| yolo11n_b4 | 640×640 | 2026-06-08 | 00c7eff3 | eco (intra=5 inter=2) | 300 | 521.1 | 541.2 | 37.95 | **+1.21x** |
| yolo11n_b4 | 640×640 | 2026-06-08 | 00c7eff3 | power (intra=12 inter=6) | 300 | 457.7 | 470.1 | 43.18 | **+1.38x** |
| yolo11n_320_b4 | 320×320 | 2026-06-08 | 4919f1c7 | n/a (4 threads) | 100 | 121.4 | 128.5 | 160.3 | baseline |
| yolo11n_320_b4 | 320×320 | 2026-06-08 | 00c7eff3 | eco (intra=5 inter=2) | 300 | 98.9 | 103.7 | 199.3 | **+1.24x** |
| yolo11n_320_b4 | 320×320 | 2026-06-08 | 00c7eff3 | power (intra=12 inter=6) | 300 | 93.4 | 97.4 | 211.6 | **+1.32x** |
| yolo11s_b4 | 640×640 | 2026-06-08 | 4919f1c7 | n/a (4 threads) | 100 | 1655.7 | 1823.2 | 11.81 | baseline |
| yolo11s_b4 | 640×640 | 2026-06-08 | 00c7eff3 | eco (intra=5 inter=2) | 300 | 1343.8 | 1381.0 | 14.75 | **+1.25x** |
| yolo11s_b4 | 640×640 | 2026-06-08 | 00c7eff3 | power (intra=12 inter=6) | 300 | 1130.5 | 1149.8 | 17.53 | **+1.48x** |
| yolo11s_320_b4 | 320×320 | 2026-06-08 | 4919f1c7 | n/a (4 threads) | 100 | 343.0 | 395.6 | 57.05 | baseline |
| yolo11s_320_b4 | 320×320 | 2026-06-08 | 00c7eff3 | eco (intra=5 inter=2) | 300 | 277.6 | 285.1 | 71.40 | **+1.25x** |
| yolo11s_320_b4 | 320×320 | 2026-06-08 | 00c7eff3 | power (intra=12 inter=6) | 300 | 212.0 | 217.4 | 93.54 | **+1.64x** |

## Classification — batch 1

| Model | Input | Date | Commit | `perf_mode` | Runs | Avg (ms) | p95 (ms) | Throughput (img/s) | vs baseline |
|---|---|---|---|---|---:|---:|---:|---:|---:|
| mobilenetv2 | 224×224 | 2026-06-08 | 4919f1c7 | n/a (4 threads) | 100 | 54.2 | 65.6 | 90.12 | baseline |
| mobilenetv2 | 224×224 | 2026-06-08 | 00c7eff3 | eco (intra=5 inter=2) | 300 | 49.4 | 51.4 | 100.32 | **+1.11x** |
| mobilenetv2 | 224×224 | 2026-06-08 | 00c7eff3 | power (intra=12 inter=6) | 300 | 81.4 | 83.3 | 60.90 | 0.68x ⚠️ |
| resnet18 | 224×224 | 2026-06-08 | 4919f1c7 | n/a (4 threads) | 100 | 65.3 | 85.2 | 74.73 | baseline |
| resnet18 | 224×224 | 2026-06-08 | 00c7eff3 | eco (intra=5 inter=2) | 300 | 52.4 | 53.7 | 94.50 | **+1.26x** |
| resnet18 | 224×224 | 2026-06-08 | 00c7eff3 | power (intra=12 inter=6) | 300 | 56.4 | 58.2 | 87.80 | **+1.17x** |
| squeezenet | 224×224 | 2026-06-08 | 4919f1c7 | n/a (4 threads) | 100 | 12.0 | 12.8 | 402.8 | baseline |
| squeezenet | 224×224 | 2026-06-08 | 00c7eff3 | eco (intra=5 inter=2) | 300 | 11.5 | 11.8 | 426.9 | **+1.06x** |
| squeezenet | 224×224 | 2026-06-08 | 00c7eff3 | power (intra=12 inter=6) | 300 | 11.5 | 11.8 | 426.7 | **+1.06x** |

---

## Notes

### Optimization history (4919f1c7 → 00c7eff3)

Three changes landed between the baseline and the current commit:

1. **Semaphore-based worker loop** (replaces busy-wait spin) — eliminates idle CPU burn between requests.
2. **`perf_mode` config key** (`eco` = 40% of cores, `power` = 90%) — replaces the hardcoded 4-thread default and adapts to the machine.
3. **2:1 intra:inter thread split** — budget per model is divided as `intra = budget*2/3`, `inter = intra/2`, keeping total threads within the `perf_mode` cap.

On this machine (20 cores): eco → intra=5 inter=2; power → intra=12 inter=6.

### Current results vs baseline

Every model improves in at least one mode. Power mode is consistently best for batch-4 workloads (+1.32–1.64x). Eco mode is competitive or better for batch-1 workloads, and for small lightweight models (squeezenet, yolo11n_320) eco and power converge.

**One outstanding regression:** mobilenetv2 in power mode (60.9 img/s vs 90.1 baseline, 0.68x). Its depthwise-convolution architecture has limited intra-op parallelism, so 12 intra threads introduces coordination overhead that outweighs the gain. Eco mode (5 intra threads) avoids this and beats the baseline at 100 img/s. Use eco mode for mobilenetv2 workloads.
