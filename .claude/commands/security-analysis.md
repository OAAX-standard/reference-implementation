---
name: security-analysis
description: Run a security audit of the OAAX reference implementation — dependencies, C++ memory safety, Python input validation, CI/CD secrets, Docker hygiene
model: sonnet
effort: high
---

Invoke the `security-analyzer` agent to audit the repository's security posture.

The agent will check:
1. Dependency vulnerabilities (ONNX Runtime, Python packages, Docker base image, Dependabot alerts)
2. C++ memory safety issues in the runtime library
3. Python input validation in the conversion toolchain
4. CI/CD secret handling and GitHub Actions security
5. Docker image hygiene
6. Personal data and sensitive information (emails, tokens, internal paths, IP addresses) that shouldn't be in the repository

Findings are reported by severity (Critical → High → Medium → Low) with specific file locations and fix recommendations.

Run this:
- After any dependency or CI change
- When Dependabot alerts appear
- Periodically as part of release preparation
