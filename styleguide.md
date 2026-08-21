# Styleguide & Architecture Standards

## C++ Standards
- **Language Standard**: Modern C++17 (`-std=c++17`).
- **Compiler Support**: Clang++, GCC, Apple Clang, MSVC.
- **Dependencies**: Standard Library (STL) + System `libcurl` (`-lcurl`). Zero third-party package managers required.
- **Header Files**: `.hpp` with `#pragma once`.
- **Naming Conventions**:
  - Types / Classes / Structs: `PascalCase` (e.g. `ChatSession`, `ConfigManager`, `LLMProvider`)
  - Functions / Methods: `snake_case` (e.g. `build_request`, `parse_response`)
  - Variables / Members: `snake_case` with trailing underscore for private members (e.g. `api_key_`, `provider_name_`)
  - Constants / Enums: `kPascalCase` or `UPPER_SNAKE_CASE` (e.g. `Role::User`, `Role::Assistant`, `DEFAULT_TIMEOUT_SEC`)

## Code Quality & Architecture Rules
- **Line Count Limit**: No component or function may exceed 150 lines. Break down complex routines into modular helper functions.
- **Strict Typing**: Strongly typed enums (`enum class`), explicit type conversions, avoid `void*` or untyped casts.
- **Memory Management**: RAII everywhere. Use `std::unique_ptr`, `std::shared_ptr`, and standard RAII wrappers for curl handles and file pointers.
- **Error Handling**: Graceful error propagation using `std::optional`, `std::expected` / result structs, and descriptive error messages rather than uncaught panics.

## Terminal UI & UX
- **ANSI Color Palette**:
  - Prompt: Cyan / Bright Blue
  - Assistant Output: Normal / White (or syntax highlighted code blocks)
  - Info / Metadata: Dim / Gray
  - Warnings: Yellow
  - Errors: Bold Red
  - Success / Set: Green
- **TTY Detection**: Automatically disable ANSI escape sequences when standard output is not a TTY (pipes, redirected to file).
- **Streaming**: Smooth token streaming directly to stdout with immediate flushing (`std::cout.flush()`).
