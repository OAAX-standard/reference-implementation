---
name: analyze-ux
description: Analyze and improve the developer experience and user-friendliness of the OAAX repository
model: sonnet
effort: high
---

Invoke the `ux-analyzer` agent to evaluate the repository from a first-time developer's perspective.

The agent will:
1. Evaluate onboarding experience (README, setup scripts, build instructions)
2. Assess C API clarity and documentation completeness
3. Check repository health (CHANGELOG, CI badges, .gitignore coverage)
4. Produce a prioritized list of findings with specific suggestions

High-priority findings will be fixed immediately (with your approval). Medium and low-priority suggestions will be presented for review.

Run this periodically (e.g., after a significant feature addition) or when onboarding a new contributor.
