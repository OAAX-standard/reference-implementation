# C++ Style Rules

## Naming

- Functions and variables: `snake_case`
- Classes and structs: `PascalCase`
- Constants and macros: `UPPER_SNAKE_CASE`
- Private members: `snake_case` (no prefix/suffix)

## Structure

- Small, focused functions — single responsibility
- Early returns over nested conditionals
- Avoid deeply nested blocks; extract into helpers instead

## Comments

Only comment when the **why** is non-obvious — hidden constraints, subtle invariants, workarounds. Never comment what the code already says.

## Error Handling

Return integer status codes (0 = success, non-zero = error) matching the existing C API pattern. Store error strings via `setError()` for retrieval by `runtime_error_message()`.

## Headers

Keep `runtime_core.hpp` as a pure C interface (`extern "C"`). C++ internals stay in `.cpp` files or `runtime_utils.hpp`.

## Includes

Order: standard library, then third-party (onnxruntime, spdlog), then project headers. One blank line between groups.
