# Drast Semantic Enforcement Progress

Updated: 2026-05-15

## Current status

The bootstrap semantic checker now enforces a focused conformance slice from `SEMANTICS.md` while keeping the legacy C++ backend test suite passing. This is not a claim that Phase 1 or Phase 2 is complete. Runtime semantics, profiles, and borrow checking still have major work outstanding.

## Verified checks

- `tests/conformance/run_conformance.sh`: 48 passed, 0 failed.
- `tests/run_tests.sh`: 155 passed, 0 failed.
- `xmake test`: 155 passed, 0 failed.

## Enforced diagnostics

- E0010: immutable bindings and immutable function parameters cannot be reassigned unless declared with `mut`.
- E0020: array literals must be homogeneous.
- E0021: function parameters require explicit type annotations.
- E0022: module-level globals require explicit type annotations in the strict semantic checker.
- E0023: incompatible assignments are rejected instead of relying on implicit conversion.
- E0024: `any` is rejected as a type annotation.
- E0030: legacy exception-style `try` / `catch` blocks are rejected.
- E0031: prefix `try` requires a `maybe T` or `Result[T, E]` operand and a compatible enclosing return type.
- E0040: field access on `maybe T` is rejected until the value is explicitly unwrapped.
- E0041: `maybe T` cannot be passed or assigned where `T` is required.
- E0042: legacy `nil` is rejected in safe Drast; use `None`.
- E0050: contextually typed integer literals are checked for target range overflow.
- E0051: implicit cross-width integer conversion is rejected.
- E0052: implicit signed/unsigned integer conversion is rejected.
- E0053: legacy integer/float names such as `int`, `uint`, `float`, and `double` are rejected by the strict checker.
- E0060: sibling newtypes are distinct even when their representation type matches.
- E0061: raw representation values are not accepted where a newtype is expected, and newtypes do not flow back to raw values implicitly.
- E0070: integer literals do not initialize float bindings without an explicit cast, and float width mismatches are rejected.
- E0071: implicit int/float conversion is rejected.
- E0080: `if` and `while` conditions require `bool`.
- E0081: `maybe T` is not truthy.
- E0082: assignment in condition position is rejected as not-an-expression.
- E0090: strings cannot be integer-indexed in safe code.
- E0091: `c'...'` must contain exactly one scalar according to the current literal checker.
- E0092: mutating string methods require a mutable receiver binding.

## Pending semantic work

- E0001 and E0002 are deferred to the borrow checker. `~T` and `~expr` are still structural pointer/reference forms, with `TODO(borrow-check)` markers at the unchecked sites.
- E0011 and E0012 are also deferred to the borrow checker; mutable borrow aliasing is not enforced yet.
- Integer overflow runtime checks are not implemented. The checker recognizes fixed-width types and wrapping/saturating operator tokens, but debug/profile-dependent checked arithmetic and division/modulo-by-zero panic lowering still need backend work.
- `&+`, `&-`, `&*`, `&<<`, `|+|`, `|-|`, and `|*|` are lexed and typechecked as integer arithmetic, but their C++ emission semantics are not yet specialized.
- `checked_add`, `checked_sub`, and `checked_mul` are not implemented.
- `maybe T` is recognized by the checker with `Some[T]`, `None`, `.value_or[...]`, `->force[]`, and `->force_with[...]`; full `match` and `if let` refinement is not implemented yet.
- `Result[T, E]`, `Ok[T]`, `Err[E]`, and prefix `try` are recognized by the checker; full error conversion and propagation semantics are incomplete.
- Newtypes are enforced in the semantic checker, but C++ backend layout/codegen support is still bootstrap-level.
- `#profile(default|embedded|safety_critical)` is not implemented.
- Profile-controlled panic strategy is not implemented.
- Safety-critical float finite-guard warnings are not implemented.
- String UTF-8 APIs are surfaced to the checker, but runtime/library completeness and Unicode scalar counting need deeper implementation. The current `c'...'` checker is still byte-length based for non-ASCII scalars.
- Function return type policy remains an open syntax/spec issue because legacy Drast uses "no comma" to mean `void`.
- The return-type ambiguity above is recorded in `SEMANTICS.md` §1.3 Open questions.
- The direct conformance tests are Drast-level tests. They intentionally pass when the semantic checker accepts a source file, independent of whether the current C++ backend can compile that source.

## Source contradictions found

- `SYNTAX.md` documented `int`, `float`, and `double` as primitive type names. `SEMANTICS.md` replaces them with fixed-width integer types and `f32`/`f64`.
- `SYNTAX.md` documented single-quoted one-character literals as `char`; `SEMANTICS.md` makes all `'...'` literals strings and reserves `c'...'` for `char`.
- `SYNTAX.md` documented `nil` as optional-null. `SEMANTICS.md` removes null from safe code and uses `None`.
- `SYNTAX.md` documented C++ `try` / `catch` blocks. `SEMANTICS.md` uses `Result[T, E]` and prefix `try` propagation only.
- `SYNTAX.md` documented optional `.value` and implicit optional assignment unwrapping. `SEMANTICS.md` requires explicit unwrap surfaces.
- The legacy compiler and test suite still contain many bootstrap-era examples that rely on relaxed numeric and optional rules. Non-strict package builds suppress the new semantic diagnostics so existing backend coverage can keep running while the direct conformance suite enforces the spec.

## Test suite additions

`tests/conformance/` now contains positive and negative Drast-level tests for each enforced diagnostic in this slice. Negative tests assert the diagnostic code from the filename, not the wording.
