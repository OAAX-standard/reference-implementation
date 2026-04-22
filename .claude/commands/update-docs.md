---
name: update-docs
description: Update all documentation to reflect recent code changes in the OAAX reference implementation
model: sonnet
effort: medium
---

Invoke the `doc-updater` agent to sync all documentation with the current codebase.

The agent will:
1. Diff the current branch against `main` to identify what changed
2. Update the relevant README files, inline comments, and CHANGELOG
3. Keep `.claude/CLAUDE.md` accurate if build commands or architecture changed
4. Flag any public API behavior changes that need prominent documentation

Run this after `/implement` and before `/create-pr`.
