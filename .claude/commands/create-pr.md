---
name: create-pr
description: Create a GitHub pull request for the current branch and monitor CI workflows
model: sonnet
effort: medium
---

Invoke the `pr-manager` agent to create and track the pull request.

The agent will:
1. Run the pre-PR checklist (clean working tree, passing local build, CHANGELOG entry)
2. Create the PR via `gh pr create` with a properly formatted title and description
3. Monitor GitHub Actions (`build-runtime` and `build-toolchain` workflows) until they complete
4. Report the PR URL and final CI status

If CI fails, the agent will retrieve the failure logs and help diagnose the issue.

Do not run this until `/implement`, `/check-style`, and `/update-docs` are complete.
