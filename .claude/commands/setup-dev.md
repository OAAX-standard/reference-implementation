---
name: setup-dev
description: Set up or validate the OAAX development environment on this machine
model: sonnet
effort: medium
---

Invoke the `dev-setup` agent to audit and configure the development environment for this machine.

The agent will:
1. Check all required toolchains and tools
2. Install anything missing via `scripts/setup-env.sh`
3. Run a test build to confirm everything works
4. Report what was installed and what (if anything) needs manual action
