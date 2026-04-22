---
name: implement
description: Implement a feature or fix in the OAAX runtime library or conversion toolchain
model: sonnet
effort: high
---

Invoke the `implementer` agent to carry out the code changes.

If a plan exists in `.claude/plans/`, the agent will use it. For straightforward bug fixes, the agent can proceed without a plan — but for anything non-trivial, run `/plan-feature` first.

The agent will:
1. Read and follow the implementation plan (if present)
2. Apply changes to C++ and/or Python source files
3. Run a build to verify the changes compile and link correctly
4. Report what changed and confirm the build result

After implementation, the typical next steps are `/check-style`, `/write-tests`, `/update-docs`, then `/create-pr`.
