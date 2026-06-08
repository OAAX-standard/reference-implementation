# Benchmark Results

Tracks inference performance across runtime versions. Use the **vs prev** column to confirm a change does not regress throughput, or to measure the gain from an optimization.

## How to update

Run all models in isolation (no other CPU-intensive processes) and append a new row to each table. Set `PERF=eco` or `PERF=power` as needed:

```bash
cd tests/runtime/build
PERF=eco  # or power

# YOLO models — batch 1
for spec in "yolo11n images 640" "yolo11n_320 images 320" "yolo11s images 640" "yolo11s_320 images 320" "yolov8n images 640"; do
  name=$(echo $spec | awk '{print $1}')
  iname=$(echo $spec | awk '{print $2}')
  sz=$(echo $spec | awk '{print $3}')
  echo "=== $name ==="
  LD_LIBRARY_PATH=. ./yolo_test ../../test_models/simplified/${name}-simplified.onnx \
    --warmup 5 --runs 300 --input-name $iname --imgsz $sz --perf-mode $PERF 2>/dev/null | grep -E "Avg|p95|Throughput"
done

# YOLO models — batch 4
for spec in "yolo11n_b4 images 640" "yolo11n_320_b4 images 320" "yolo11s_b4 images 640" "yolo11s_320_b4 images 320"; do
  name=$(echo $spec | awk '{print $1}')
  iname=$(echo $spec | awk '{print $2}')
  sz=$(echo $spec | awk '{print $3}')
  echo "=== $name ==="
  LD_LIBRARY_PATH=. ./yolo_test ../../test_models/simplified/${name}-simplified.onnx \
    --warmup 5 --runs 300 --input-name $iname --imgsz $sz --batch 4 --perf-mode $PERF 2>/dev/null | grep -E "Avg|p95|Throughput"
done

# Classification models
LD_LIBRARY_PATH=. ./yolo_test ../../test_models/simplified/mobilenetv2-simplified.onnx \
  --warmup 5 --runs 300 --input-name data   --imgsz 224 --no-validate --perf-mode $PERF 2>/dev/null | grep -E "Avg|p95|Throughput"
LD_LIBRARY_PATH=. ./yolo_test ../../test_models/simplified/resnet18-simplified.onnx \
  --warmup 5 --runs 300 --input-name data   --imgsz 224 --no-validate --perf-mode $PERF 2>/dev/null | grep -E "Avg|p95|Throughput"
LD_LIBRARY_PATH=. ./yolo_test ../../test_models/simplified/squeezenet-simplified.onnx \
  --warmup 5 --runs 300 --input-name data_0 --imgsz 224 --no-validate --perf-mode $PERF 2>/dev/null | grep -E "Avg|p95|Throughput"
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
| yolo11n | 640×640 | 2026-06-08 | 2bce63a1 | power (18 threads) | 300 | 118.0 | 200.5 | 41.66 | 0.97x |
| yolo11n | 640×640 | 2026-06-08 | 6650641d | eco (intra=5 inter=2) | 300 | 106.3 | 110.4 | 46.45 | **+1.09x** |
| yolo11n | 640×640 | 2026-06-08 | 6650641d | power (intra=12 inter=6) | 300 | 105.1 | 107.2 | 47.08 | **+1.13x** |
| yolo11n_320 | 320×320 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 33.2 | 35.6 | 146.6 | baseline |
| yolo11n_320 | 320×320 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 40.0 | 45.1 | 122.8 | 0.84x |
| yolo11n_320 | 320×320 | 2026-06-08 | 2bce63a1 | power (18 threads) | 300 | 57.6 | 101.9 | 86.0 | 0.70x ⚠️ |
| yolo11n_320 | 320×320 | 2026-06-08 | 6650641d | eco (intra=5 inter=2) | 300 | 28.4 | 30.6 | 173.6 | **+1.41x** |
| yolo11n_320 | 320×320 | 2026-06-08 | 6650641d | power (intra=12 inter=6) | 300 | 27.9 | 29.0 | 176.6 | **+2.05x** |
| yolo11s | 640×640 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 343.5 | 387.4 | 14.25 | baseline |
| yolo11s | 640×640 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 298.3 | 486.2 | 16.62 | **+1.17x** |
| yolo11s | 640×640 | 2026-06-08 | 2bce63a1 | power (18 threads) | 300 | 260.4 | 449.1 | 19.03 | **+1.15x** |
| yolo11s | 640×640 | 2026-06-08 | 6650641d | eco (intra=5 inter=2) | 300 | 268.3 | 278.3 | 18.46 | **+1.11x** |
| yolo11s | 640×640 | 2026-06-08 | 6650641d | power (intra=12 inter=6) | 300 | 224.3 | 231.1 | 22.10 | **+1.16x** |
| yolo11s_320 | 320×320 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 79.0 | 85.1 | 61.81 | baseline |
| yolo11s_320 | 320×320 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 63.2 | 78.1 | 78.24 | **+1.27x** |
| yolo11s_320 | 320×320 | 2026-06-08 | 2bce63a1 | power (18 threads) | 300 | 70.2 | 76.4 | 70.47 | 0.90x |
| yolo11s_320 | 320×320 | 2026-06-08 | 6650641d | eco (intra=5 inter=2) | 300 | 71.8 | 75.1 | 68.97 | 0.88x |
| yolo11s_320 | 320×320 | 2026-06-08 | 6650641d | power (intra=12 inter=6) | 300 | 67.1 | 69.0 | 73.77 | **+1.05x** |
| yolov8n | 640×640 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 106.5 | 137.7 | 46.40 | baseline |
| yolov8n | 640×640 | 2026-06-08 | 2bce63a1 | power (18 threads) | 300 | 108.7 | 118.4 | 45.45 | 0.98x |
| yolov8n | 640×640 | 2026-06-08 | 6650641d | eco (intra=5 inter=2) | 300 | 110.0 | 113.1 | 44.93 | 0.97x |
| yolov8n | 640×640 | 2026-06-08 | 6650641d | power (intra=12 inter=6) | 300 | 113.8 | 117.4 | 43.50 | 0.96x |

## YOLO — batch 4

| Model | Input | Date | Commit | `perf_mode` | Runs | Avg (ms) | p95 (ms) | Throughput (img/s) | vs prev |
|---|---|---|---|---|---:|---:|---:|---:|---:|
| yolo11n_b4 | 640×640 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 622.6 | 694.4 | 31.34 | baseline |
| yolo11n_b4 | 640×640 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 776.2 | 1176.0 | 25.43 | 0.81x ⚠️ |
| yolo11n_b4 | 640×640 | 2026-06-08 | 2bce63a1 | power (18 threads) | 300 | 446.8 | 478.0 | 43.89 | **+1.73x** |
| yolo11n_b4 | 640×640 | 2026-06-08 | 6650641d | eco (intra=5 inter=2) | 300 | 521.1 | 541.2 | 37.95 | **+1.49x** |
| yolo11n_b4 | 640×640 | 2026-06-08 | 6650641d | power (intra=12 inter=6) | 300 | 457.7 | 470.1 | 43.18 | 0.98x |
| yolo11n_320_b4 | 320×320 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 121.4 | 128.5 | 160.3 | baseline |
| yolo11n_320_b4 | 320×320 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 252.3 | 394.4 | 78.53 | 0.49x ⚠️ |
| yolo11n_320_b4 | 320×320 | 2026-06-08 | 2bce63a1 | power (18 threads) | 300 | 98.5 | 106.2 | 200.5 | **+2.55x** |
| yolo11n_320_b4 | 320×320 | 2026-06-08 | 6650641d | eco (intra=5 inter=2) | 300 | 98.9 | 103.7 | 199.3 | **+2.54x** |
| yolo11n_320_b4 | 320×320 | 2026-06-08 | 6650641d | power (intra=12 inter=6) | 300 | 93.4 | 97.4 | 211.6 | **+1.06x** |
| yolo11s_b4 | 640×640 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 1655.7 | 1823.2 | 11.81 | baseline |
| yolo11s_b4 | 640×640 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 1401.4 | 2058.4 | 14.13 | **+1.20x** |
| yolo11s_b4 | 640×640 | 2026-06-08 | 2bce63a1 | power (18 threads) | 300 | 1168.1 | 1244.2 | 16.92 | **+1.20x** |
| yolo11s_b4 | 640×640 | 2026-06-08 | 6650641d | eco (intra=5 inter=2) | 300 | 1343.8 | 1381.0 | 14.75 | **+1.04x** |
| yolo11s_b4 | 640×640 | 2026-06-08 | 6650641d | power (intra=12 inter=6) | 300 | 1130.5 | 1149.8 | 17.53 | **+1.04x** |
| yolo11s_320_b4 | 320×320 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 343.0 | 395.6 | 57.05 | baseline |
| yolo11s_320_b4 | 320×320 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 288.5 | 474.4 | 68.68 | **+1.20x** |
| yolo11s_320_b4 | 320×320 | 2026-06-08 | 2bce63a1 | power (18 threads) | 300 | 208.9 | 224.1 | 94.90 | **+1.38x** |
| yolo11s_320_b4 | 320×320 | 2026-06-08 | 6650641d | eco (intra=5 inter=2) | 300 | 277.6 | 285.1 | 71.40 | **+1.04x** |
| yolo11s_320_b4 | 320×320 | 2026-06-08 | 6650641d | power (intra=12 inter=6) | 300 | 212.0 | 217.4 | 93.54 | 0.99x |

## Classification — batch 1

| Model | Input | Date | Commit | `perf_mode` | Runs | Avg (ms) | p95 (ms) | Throughput (img/s) | vs prev |
|---|---|---|---|---|---:|---:|---:|---:|---:|
| mobilenetv2 | 224×224 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 54.2 | 65.6 | 90.12 | baseline |
| mobilenetv2 | 224×224 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 105.6 | 145.7 | 46.88 | 0.52x ⚠️ |
| mobilenetv2 | 224×224 | 2026-06-08 | 2bce63a1 | power (18 threads) | 300 | 83.7 | 89.3 | 59.20 | **+1.26x** |
| mobilenetv2 | 224×224 | 2026-06-08 | 6650641d | eco (intra=5 inter=2) | 300 | 49.4 | 51.4 | 100.32 | **+2.14x** |
| mobilenetv2 | 224×224 | 2026-06-08 | 6650641d | power (intra=12 inter=6) | 300 | 81.4 | 83.3 | 60.90 | **+1.03x** |
| resnet18 | 224×224 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 65.3 | 85.2 | 74.73 | baseline |
| resnet18 | 224×224 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 110.4 | 171.0 | 44.92 | 0.60x ⚠️ |
| resnet18 | 224×224 | 2026-06-08 | 2bce63a1 | power (18 threads) | 300 | 54.3 | 58.6 | 90.97 | **+2.02x** |
| resnet18 | 224×224 | 2026-06-08 | 6650641d | eco (intra=5 inter=2) | 300 | 52.4 | 53.7 | 94.50 | **+2.10x** |
| resnet18 | 224×224 | 2026-06-08 | 6650641d | power (intra=12 inter=6) | 300 | 56.4 | 58.2 | 87.80 | 0.97x |
| squeezenet | 224×224 | 2026-06-08 | db223e77 | n/a (4 threads) | 100 | 12.0 | 12.8 | 402.8 | baseline |
| squeezenet | 224×224 | 2026-06-08 | 2bce63a1 | eco (8 threads) | 300 | 27.0 | 50.0 | 181.8 | 0.45x ⚠️ |
| squeezenet | 224×224 | 2026-06-08 | 2bce63a1 | power (18 threads) | 300 | 11.8 | 13.8 | 416.5 | **+2.29x** |
| squeezenet | 224×224 | 2026-06-08 | 6650641d | eco (intra=5 inter=2) | 300 | 11.5 | 11.8 | 426.9 | **+2.35x** |
| squeezenet | 224×224 | 2026-06-08 | 6650641d | power (intra=12 inter=6) | 300 | 11.5 | 11.8 | 426.7 | **+1.02x** |

---

## Notes

### 2bce63a1 — perf_mode + semaphore changes

**Power mode (18 threads) vs eco mode (8 threads) takeaways:**

- **Batch 4 models benefit enormously from power mode** — yolo11n_b4: +1.73x, yolo11n_320_b4: +2.55x, yolo11s_320_b4: +1.38x. Larger batches have more parallelisable work, so additional threads pay off.
- **Classification models recover with power mode** — resnet18: +2.02x (now surpassing the 4-thread baseline), squeezenet: +2.29x (fully recovered to 416 img/s). These models are medium-complexity; 8 eco threads was insufficient but 18 power threads hits the sweet spot.
- **Small/fast batch-1 YOLO models are hurt by power mode** — yolo11n_320 (batch 1) drops to 86 img/s vs 123 with eco. These models compute so fast that 18-thread coordination overhead exceeds parallelism gains. For this class of model, fewer threads (4–8) remain optimal.
- **mobilenetv2 is still below baseline** at 59.2 img/s vs 90.12 original, even in power mode. This is the one model where over-threading hurts across all modes; likely needs a model-complexity heuristic or explicit per-model cap.

**Eco mode ⚠️ regressions on batch-4 YOLO** (yolo11n_b4 0.81x, yolo11n_320_b4 0.49x) were measurement artifacts — zombie processes were running during that benchmark session. Power mode (clean run) shows +1.73x and +2.55x on the same models.

**Recommendation:** use `power` mode for batch inference workloads and for larger models; use `eco` mode (or a future `--num-threads` override) for small/lightweight models running batch-1.

**Positive signals:** All previously-failing lifecycle and multi-model C++ tests now pass (they were timing out with the old 4-thread default). Large YOLO models (yolo11n 640, yolo11s 640, yolo11s_b4, yolo11s_320_b4) all improved vs the original baseline in both modes.

### 6650641d — inter_threads = max(1, intra/2) within perf_mode budget

Thread budget per model is now split 2:1 (intra:inter): `intra = max(1, budget*2/3)`, `inter = max(1, intra/2)`.
On 20 cores: eco → intra=5 inter=2; power → intra=12 inter=6.

**Key improvements over 2bce63a1 (where inter was always 1):**

- **Classification eco mode fully recovered** — mobilenetv2: +2.14x (100 img/s, now beats the 4-thread baseline), resnet18: +2.10x (94.5 img/s), squeezenet: +2.35x (427 img/s). The inter-op threads expose graph-level parallelism these models have.
- **yolo11n_320 eco: +1.41x** — previously regressed vs baseline, now at 174 img/s vs original 147.
- **yolo11n_320 power: +2.05x** — from 86 to 177 img/s; fixing the over-threading problem from 2bce63a1.
- **All YOLO batch-4 models hold or improve** — eco mode numbers are now clean (no zombie processes).
- **yolov8n and yolo11n_b4 power are flat** (~0.97-0.98x) — these models were already near-optimal.

**Outstanding issue:** mobilenetv2 in power mode (60.9 img/s) is still below its 4-thread baseline (90.1 img/s). This model appears to prefer fewer intra-op threads regardless of available budget — possibly due to its depthwise-conv heavy architecture which has limited data parallelism per op.
