# PR Guidelines

## Title Format

`<type>: <short description>` — e.g., `feat: add number_of_threads init arg` or `fix: correct AARCH64 alignment`.

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

Before merging, both GitHub Actions workflows must pass:
- `build-runtime.yml` (Linux + Windows builds)
- `build-toolchain.yml` (Docker image build)

## Branch Naming

`<type>/<short-description>` — e.g., `feat/add-batch-inference` or `fix/windows-dll-path`.
