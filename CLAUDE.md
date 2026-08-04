# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Working Style

- Question requirements and assumptions before acting — don't just execute blindly.
- Challenge decisions that look wrong or suboptimal, even if not asked.
- Propose alternatives when a better approach exists.
- Ask clarifying questions upfront rather than discovering ambiguity mid-task.

## Project Overview

OAAX Reference Implementation — two components that together form an AI model deployment pipeline:

1. **Conversion Toolchain**: Dockerized Python tool that optimizes ONNX models (via onnxsim) for deployment.
2. **Runtime Library**: C++ shared library wrapping ONNX Runtime, exposing a minimal C API for loading and running optimized models.

Current version: `VERSION` file at the repo root.

## Build Commands

### Environment Setup (run once on a fresh machine)
```bash
sudo bash scripts/setup-env.sh
```
Installs cross-compilation toolchains to `/opt/` and CMake 3.31.7.

### Runtime Library (Linux)
```bash
bash runtime-library/build-runtimes.sh            # both X86_64 and AARCH64
bash runtime-library/build-runtimes.sh X86_64     # single platform
bash runtime-library/build-runtimes.sh AARCH64
```
Outputs: `runtime-library/artifacts/runtime-library-{ARCH}.tar.gz`

### Runtime Library (Windows)
```bat
cd runtime-library
.\build-runtime.bat
```

### Conversion Toolchain
```bash
bash conversion-toolchain/build-toolchain.sh
```
Requires Docker. Output: `conversion-toolchain/artifacts/oaax-cpu-toolchain.tar`

## Architecture

### Two-Stage Pipeline

```
ONNX Model → [Conversion Toolchain] → Optimized ONNX → [Runtime Library] → Inference Results
```

**Conversion Toolchain** (`conversion-toolchain/conversion_toolchain/`):
- `main.py`: CLI entry point; orchestrates the conversion flow
- `utils.py`: Wraps `onnxsim.simplify()` and computes MD5 checksums
- `logger.py`: JSON-based logging that records each conversion step

**Runtime Library** (`runtime-library/src/`):
- `runtime_core.cpp`: Implements the 9-function C API; manages ONNX Runtime sessions (N replicas per model), their worker threads, and two lock-free queues (input/output) via `moodycamel::ConcurrentQueue`
- `runtime_utils.cpp`: Type mapping between the internal `tensor_data_type` enum and ONNX element types; spdlog initialization

### C API (`runtime-library/include/oaax_runtime.h`)

```c
RuntimeStatus runtime_init(Config config);
RuntimeStatus runtime_load_models(int num_models, const ModelConfig *model_configs);
RuntimeStatus runtime_enqueue_input(int model_id, Tensors *input_tensors);
RuntimeStatus runtime_retrieve_output(int *model_id, Tensors **output_tensors, int timeout_ms);
RuntimeStatus runtime_cleanup(void);
const char *runtime_get_error(void);
const char *runtime_get_version(void);
const char *runtime_get_name(void);
const char *runtime_get_info(void);
```

`Config` is a `{length, keys[], values[]}` key-value struct. Supported init keys: `log_level` (0–6), `log_file`, `perf_mode` (`eco`|`power`), `num_intra_threads` (0 = heuristic), `num_replicas` (0 = heuristic). These keys must stay in sync with `config_get()` calls in `runtime_core.cpp` — update the header comment, `runtime-library/README.md`, and this file together. `ModelConfig` carries `file_path`, optional `model_data`/`model_size` for in-memory load, and a per-model `Config`.

### Threading Model

- `runtime_load_models()` spawns N replica worker threads per model (N from the `perf_mode` CPU budget, or the `num_replicas` override). ONNX Runtime inter-op threads are fixed at 1; throughput scales via replicas.
- `runtime_enqueue_input()` pushes to a lock-free queue; a worker wakes via semaphore, runs its session replica, and pushes output.
- `runtime_retrieve_output()` pops output (blocking up to `timeout_ms`; pass `timeout_ms=0` to poll non-blocking).

### Cross-Compilation

CMake uses GCC toolchains from `/opt/`:
- X86_64: `x86_64-unknown-linux-gnu-gcc-9.5.0` with `-march=haswell`
- AARCH64: `gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu`

Pre-built third-party libraries (ONNX Runtime 1.21.1, RE2, cpuinfo) live in `runtime-library/deps/`.

### CI/CD

`.github/workflows/` has three pipelines:
- `ci.yml`: builds (Linux X86_64/AARCH64, Windows MSVC, toolchain Docker image), then Stage 1 (conversion tests + model simplification) and Stage 2 (runtime benchmarks on Linux x86_64/arm64 and Windows).
- `lint.yml`: pre-commit — ruff, clang-format, shellcheck, hadolint.
- `delete-temporary-artifacts.yml`: S3 cleanup for old PR artifacts.

A `DCO` app check requires every commit's sign-off to match its author. Note: if a PR is `CONFLICTING` with main, `pull_request` workflows silently don't run at all — resolve the conflict first.

## Testing

```bash
git submodule update --init --recursive
uv sync --extra integration && uv pip install -e conversion-toolchain/
uv run python tests/stage1.py                              # conversion + YOLO integration
uv run python tests/stage2.py --runs 300 --csv results.csv # runtime benchmarks
```

C++ tests: `bash tests/runtime/build-tests.sh` then run binaries from `tests/runtime/build/` with `LD_LIBRARY_PATH=.`.

> `uv sync` may fail on `onnxsim` (broken sdist metadata). Fix: `pip3 download onnxsim --no-deps -d /tmp/w && uv pip install /tmp/w/onnxsim-*.whl`

## Git Workflow

- **Commit** freely after every logical set of changes — no need to ask. Always sign off commits (`git commit -s`) and include Claude as co-author. The sign-off MUST exactly match the commit author's `git config user.name`/`user.email` — never a placeholder or a different identity — or the DCO check fails (see `.claude/rules/git-signoff.md`). Every commit message must end with:
  ```
  Signed-off-by: <git config user.name> <git config user.email>
  Co-Authored-By: <the Claude model in use, e.g. Claude Fable 5> <noreply@anthropic.com>
  ```
- **Push** the current branch freely at any time.
- **PRs**: notify the maintainer before creating one, then manage it autonomously — push follow-up commits, monitor CI workflows, respond to failures.
- **Merging** is the maintainer's responsibility; never merge a PR.

## Maintenance Workflows

This repository is maintained using a set of Claude Code agents and commands in `.claude/`. The standard development cycle is:

1. `/plan-feature` — produce a written plan before touching code
2. `/implement` — execute the plan
3. `/check-style` — enforce style consistency
4. `/write-tests` — add or update tests
5. `/update-docs` — sync documentation
6. `/create-pr` — open the PR and monitor CI

Other commands available at any time:
- `/setup-dev` — configure or validate the dev environment
- `/analyze-ux` — audit developer experience and docs quality
- `/security-analysis` — audit dependencies, C++ memory safety, Python input validation, CI/CD secrets, and Docker hygiene; run after any dependency change or as part of release prep

Agents, skills, and rules live in `.claude/agents/`, `.claude/skills/`, and `.claude/rules/`.
