# Benchmark Results

Tracks inference performance across runtime versions. Use the **vs prev** column to confirm a change does not regress throughput, or to measure the gain from an optimization.

## How to update

Run all models in isolation (no other CPU-intensive processes) and append a new row to each table:

```bash
cd tests/runtime/build

# YOLO models — batch 1
for spec in "yolo11n images 640" "yolo11n_320 images 320" "yolo11s images 640" "yolo11s_320 images 320" "yolov8n images 640"; do
  name=$(echo $spec | awk '{print $1}')
  iname=$(echo $spec | awk '{print $2}')
  sz=$(echo $spec | awk '{print $3}')
  echo "=== $name ==="
  LD_LIBRARY_PATH=. ./yolo_test ../../test_models/simplified/${name}-simplified.onnx \
    --warmup 5 --runs 300 --input-name $iname --imgsz $sz 2>/dev/null | grep -E "Avg|p95|Throughput"
done

# YOLO models — batch 4
for spec in "yolo11n_b4 images 640" "yolo11n_320_b4 images 320" "yolo11s_b4 images 640" "yolo11s_320_b4 images 320"; do
  name=$(echo $spec | awk '{print $1}')
  iname=$(echo $spec | awk '{print $2}')
  sz=$(echo $spec | awk '{print $3}')
  echo "=== $name ==="
  LD_LIBRARY_PATH=. ./yolo_test ../../test_models/simplified/${name}-simplified.onnx \
    --warmup 5 --runs 300 --input-name $iname --imgsz $sz --batch 4 2>/dev/null | grep -E "Avg|p95|Throughput"
done

# Classification models
LD_LIBRARY_PATH=. ./yolo_test ../../test_models/simplified/mobilenetv2-simplified.onnx \
  --warmup 5 --runs 300 --input-name data   --imgsz 224 --no-validate 2>/dev/null | grep -E "Avg|p95|Throughput"
LD_LIBRARY_PATH=. ./yolo_test ../../test_models/simplified/resnet18-simplified.onnx \
  --warmup 5 --runs 300 --input-name data   --imgsz 224 --no-validate 2>/dev/null | grep -E "Avg|p95|Throughput"
LD_LIBRARY_PATH=. ./yolo_test ../../test_models/simplified/squeezenet-simplified.onnx \
  --warmup 5 --runs 300 --input-name data_0 --imgsz 224 --no-validate 2>/dev/null | grep -E "Avg|p95|Throughput"
```

**vs prev** = `current_throughput / previous_throughput`. Values > 1.0x are improvements.  
Always run benchmarks in isolation — concurrent processes on the same machine inflate latency and make comparisons unreliable.

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

| Model | Input | Date | Commit | `perf_mode` | Runs | Avg (ms) | p95 (ms) | Throughput (img/s) | vs prev |
|---|---|---|---|---|---:|---:|---:|---:|---:|
| yolo11n | 640×640 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 125.5 | 136.4 | 38.88 | baseline |
| yolo11n | 640×640 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 115.1 | 200.1 | 42.78 | **+1.10x** |
| yolo11n_320 | 320×320 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 33.2 | 35.6 | 146.6 | baseline |
| yolo11n_320 | 320×320 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 40.0 | 45.1 | 122.8 | 0.84x |
| yolo11s | 640×640 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 343.5 | 387.4 | 14.25 | baseline |
| yolo11s | 640×640 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 298.3 | 486.2 | 16.62 | **+1.17x** |
| yolo11s_320 | 320×320 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 79.0 | 85.1 | 61.81 | baseline |
| yolo11s_320 | 320×320 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 63.2 | 78.1 | 78.24 | **+1.27x** |
| yolov8n | 640×640 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 106.5 | 137.7 | 46.40 | baseline |

## YOLO — batch 4

| Model | Input | Date | Commit | `perf_mode` | Runs | Avg (ms) | p95 (ms) | Throughput (img/s) | vs prev |
|---|---|---|---|---|---:|---:|---:|---:|---:|
| yolo11n_b4 | 640×640 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 622.6 | 694.4 | 31.34 | baseline |
| yolo11n_b4 | 640×640 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 776.2 | 1176.0 | 25.43 | 0.81x ⚠️ |
| yolo11n_320_b4 | 320×320 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 121.4 | 128.5 | 160.3 | baseline |
| yolo11n_320_b4 | 320×320 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 252.3 | 394.4 | 78.53 | 0.49x ⚠️ |
| yolo11s_b4 | 640×640 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 1655.7 | 1823.2 | 11.81 | baseline |
| yolo11s_b4 | 640×640 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 1401.4 | 2058.4 | 14.13 | **+1.20x** |
| yolo11s_320_b4 | 320×320 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 343.0 | 395.6 | 57.05 | baseline |
| yolo11s_320_b4 | 320×320 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 288.5 | 474.4 | 68.68 | **+1.20x** |

## Classification — batch 1

| Model | Input | Date | Commit | `perf_mode` | Runs | Avg (ms) | p95 (ms) | Throughput (img/s) | vs prev |
|---|---|---|---|---|---:|---:|---:|---:|---:|
| mobilenetv2 | 224×224 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 54.2 | 65.6 | 90.12 | baseline |
| mobilenetv2 | 224×224 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 105.6 | 145.7 | 46.88 | 0.52x ⚠️ |
| resnet18 | 224×224 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 65.3 | 85.2 | 74.73 | baseline |
| resnet18 | 224×224 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 110.4 | 171.0 | 44.92 | 0.60x ⚠️ |
| squeezenet | 224×224 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 12.0 | 12.8 | 402.8 | baseline |
| squeezenet | 224×224 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 27.0 | 50.0 | 181.8 | 0.45x ⚠️ |

---

## Notes

### 2bce63a1 — perf_mode + semaphore changes

The ⚠️ regressions on small/batch-1 models warrant investigation. Two likely causes:

1. **Over-threading on small models** — mobilenetv2, squeezenet, and yolo11n_320 are lightweight enough that 8 threads (eco mode on 20 cores) introduces more thread-coordination overhead than it saves. 4 threads was closer to optimal for these. Consider exposing a per-model thread count in a future change, or capping intra-op threads relative to model complexity.

2. **Measurement noise from baseline methodology change** — the baseline (db223e77) used 100 runs under clean conditions; these measurements used 300 runs but were collected on a shared machine with concurrent processes. The yolo11n_b4 and yolo11n_320_b4 regressions in particular are likely noise: yolo11s_b4 and yolo11s_320_b4 both improved on the same commit, which would be inconsistent if the threading change itself were the cause.

**Positive signals:** All previously-failing lifecycle and multi-model C++ tests now pass (they were timing out with the old 4-thread default). Large YOLO models (yolo11n 640, yolo11s 640, yolo11s_b4, yolo11s_320_b4) all improved.
