# PR Guidelines

## Title Format

`<type>: <short description>` — e.g., `feat: add perf_mode init key` or `fix: correct AARCH64 alignment`.

Types: `feat`, `fix`, `refactor`, `docs`, `ci`, `chore`.

## Description Template

```
## Summary
- What changed and why (1-3 bullets)

## Testing
- How this was verified (build succeeded, manual test, etc.)

## Platform Impact
- Which platforms are affected (X86_64 / AARCH64 / Windows / All)

---
🤖 AI-assisted change
```

## Scope

- One logical change per PR; don't bundle unrelated fixes
- If a change touches both `runtime-library/` and `conversion-toolchain/`, that's acceptable if they're part of the same feature

## CI Requirements

Before merging, all checks must pass:
- `ci.yml` — builds (Linux, Windows, toolchain image) + Stage 1 conversion tests + Stage 2 runtime benchmarks
- `lint.yml` — pre-commit (ruff, clang-format, shellcheck, hadolint)
- `DCO` — every commit signed off with the author's own identity (see [[git-signoff]] / `.claude/rules/git-signoff.md`)

## Branch Naming

`<type>/<short-description>` — e.g., `feat/add-batch-inference` or `fix/windows-dll-path`.
