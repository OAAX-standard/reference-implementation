---
name: plan-feature
description: Plan the implementation of a new feature or significant change in the OAAX reference implementation
model: sonnet
effort: high
---

Invoke the `feature-planner` agent to create a written implementation plan.

If the user provided a feature description as an argument, pass it to the agent. Otherwise, the agent will ask clarifying questions before planning.

The agent will:
1. Clarify requirements if needed
2. Analyse the relevant source files
3. Map every file that needs to change
4. Verify the plan against architecture constraints (C API stability, platform support, deps)
5. Write the plan to `.claude/plans/<feature-name>.md`

After the plan is ready, the next step is `/implement`.
