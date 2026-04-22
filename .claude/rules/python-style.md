# Python Style Rules

## Naming

- Functions, variables, modules: `snake_case`
- Classes: `PascalCase`
- Constants: `UPPER_SNAKE_CASE`

## Structure

- Small, focused functions
- Early returns over nested conditionals
- Type hints on all function signatures
- Docstrings only for public functions, one-line preferred

## Imports

Standard library first, then third-party (onnx, onnxsim), then local. Alphabetical within each group.

## Error Handling

Raise specific exceptions with clear messages. Don't catch-and-swallow. Log errors via the existing `logger.py` JSON logger before raising.

## Compatibility

The Docker image uses Python 3.8.16. Avoid syntax or stdlib features added after 3.8.
