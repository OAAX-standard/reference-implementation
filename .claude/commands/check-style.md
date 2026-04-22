---
name: check-style
description: Check and fix style guide consistency across C++ and Python source files
model: sonnet
effort: medium
---

Invoke the `style-checker` agent to audit and fix style violations.

The agent will:
1. Check `runtime-library/src/` and `runtime-library/include/` against `.claude/rules/cpp-style.md`
2. Check `conversion-toolchain/conversion_toolchain/` against `.claude/rules/python-style.md`
3. Fix violations in-place
4. Report a summary of what was checked, what was fixed, and anything left unresolved

Pass a path as an argument to scope the check (e.g., `/check-style runtime-library/src/runtime_core.cpp`), or omit to check all source files.
