---
name: write-tests
description: Write or update tests for the OAAX runtime library and conversion toolchain
model: sonnet
effort: high
---

Invoke the `test-writer` agent to write or expand the test suite.

Note: The project currently has no test infrastructure. On first use, the agent will set it up before writing tests.

The agent will:
1. Check if test infrastructure exists; create it if not
2. Write tests covering the changed or requested functionality
3. Ensure tests run without internet access or GPU
4. Confirm the tests pass

Specify what area to test as an argument (e.g., `/write-tests C API lifecycle` or `/write-tests conversion utils`), or the agent will decide based on recent changes.
