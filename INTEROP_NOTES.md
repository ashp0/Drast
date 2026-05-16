# Drast Native C++ Interop Notes

Updated: 2026-05-15

## Remaining Gaps

- C++ macros and preprocessor conditionals are reachable only through included headers, not as first-class Drast declarations. Correct resolution: add an explicit compile-time/preprocessor interop design instead of pretending macros are ordinary functions.
- C++ overload steering still relies on emitted C++ overload resolution. Correct resolution: store imported/native signatures once Drast has a richer typed IR and report ambiguous calls before C++ compilation.
- Function pointers, lambdas, closures, and capturing callable objects do not yet have Drast syntax. Correct resolution: design closure syntax and capture rules, then lower to C++ lambdas or callable classes.
- Template non-type parameters and advanced template metaprogramming constructs are not modeled directly. Correct resolution: extend generic argument parsing beyond type-only arguments and preserve native template argument spelling.
- Native C++ namespaces are currently reached through dotted access such as `std.cout`, and type names can use `::` or dotted spelling. Correct resolution: add explicit namespace import metadata and diagnostics for unknown aliases.

## Current Interop Contract

- Header imports pass through directly: `use Arduino.h` emits `#include <Arduino.h>`, while quoted/file imports emit quoted includes.
- Standard library imports pass through directly: `use vector` emits `#include <vector>`, and `use iostream as std` emits `#include <iostream>`.
- Drast calls native C++ functions and constructors by emitting ordinary C++ calls; there is no runtime wrapper layer.
- `use drast` loads `drast_flavour.drast`, a lightweight Drast prelude whose current built-ins are lowered by the transpiler.

## Deferred Language Feature

AUDIT-017 closure support remains intentionally deferred. There is no accepted closure syntax yet, so there is no generated C++ closure output to audit in this pass.

