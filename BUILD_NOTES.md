# Drast Build System Notes

## Baseline

- `xmake` passed before the build-system changes.
- `xmake build test` passed before the build-system changes with 10 passing tests.

## Planned Touch List

- `src/main.drast` for CLI subcommands, source discovery orchestration, output layout, compile/link/run/clean, and the legacy CLI shim.
- `src/parser.drast` and `src/features/use_statements.drast` for parsing project files without recursively merging auto-discovered Drast imports.
- `src/codegen.drast` for a narrow per-translation-unit emission entry point that reuses the existing emitter helpers.
- `src/features/types.drast` and `src/features/expressions.drast` for runtime helper function recognition and return typing.
- `runtime/drast_runtime.hpp` for filesystem traversal, dependency ordering, path helpers, command execution, and compiler discovery helpers.
- `BUILD_NOTES.md` and `BOOTSTRAP.md` for build decisions and bootstrap instructions.

## Conservative Decisions

- The legacy `drast <source> -o <cpp>` transpile flow remains as a compatibility shim because the existing xmake and test scripts depend on it.
- New subcommands use per-file generated C++ under `.drast/build/src` by default, with object files under `.drast/build/obj`.
- `build -o <dir>` treats `<dir>` as a build directory, writing sources under `<dir>/src`, objects under `<dir>/obj`, and the executable under `<dir>/<entry-stem>`.
- `transpile -o <dir>` writes generated `.cpp` files directly into `<dir>`.
- Incremental transpilation is mtime-based per source file. If a shared declaration changes in one file, unchanged source files are not rewritten; `clean` is the explicit escape hatch for rebuilding every generated translation unit.
- The runtime exposes global compatibility wrappers for new filesystem/process helpers so the checked-in seed compiler can bootstrap sources that call those helpers before it knows they are `drast::` runtime functions.
