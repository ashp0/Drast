# Drast Refactor Notes

## Prelude Boundary

- `use drast` is the opt-in boundary for Drast-flavoured helper idioms.
- `use std` is treated as native C++ standard-library interop, not as an implicit prelude. Compiler-owned sources that need Drast helper idioms say `use drast` explicitly.
- A source file with no `use` statements receives only the C++ required by the declarations and expressions it actually contains — verified by `cli_bare_no_prelude` in `tests/run_tests.sh`.

## Backend Strings

- The C++ backend serializes AST nodes into C++ text. The cleanup target is "no hardcoded helper/runtime C++ fragments in codegen", not "no string output in a text backend".
- Syntax lowering for `if`, `for`, `struct`, `enum`, and expression operators remains in the C++ backend until Drast has a structured C++ IR/printer. Moving that backend to typed C++ IR is a separate architecture pass.

## Prelude Expressiveness

The drast_flavour prelude expresses the helpers Drast can cleanly write today (print, println, charCode, isAlpha, isDigit, isWhitespace, plus a handful of pure-Drast variants). The remaining helpers stay in the inline `__drt::` support block that `Codegen::emitSupportBlock` produces only when a unit references those helpers; that block is the staging area while the language grows the features needed to host the rest in pure Drast.

Concrete language features that the prelude needs before more helpers can move:

- Variadic generics so `print(a, b, c)` can be defined in Drast.
- Zero-argument method-call inference so `ss.str` lowers to `ss.str()` for stream-style accessors.
- Multi-segment dotted namespace access so `std.filesystem.exists path` lowers to `std::filesystem::exists(path)` rather than treating `filesystem` as a member of `std`.
- `static` and `thread_local` storage qualifiers on locals so random number generators and `args()` storage can live in pure Drast.
- A stream-style `[]` constructor form so types like `std::ifstream[path]` and `std::ostringstream[]` are emitted as constructor calls.

When those features land, the inline support block shrinks one helper at a time. Each helper moved into `drast_flavour.drast` should arrive with a regression test under `tests/` that exercises `use drast` and inspects the generated CPP for the new lowering shape.

## Codegen Ordering

`Codegen::emitUnit` emits forward function declarations before methods so that any method body in a multi-unit build can call top-level functions defined elsewhere. Methods compile against the cross-unit forward declarations at the top of the file rather than depending on functions further down.
