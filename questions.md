# Drast: Analysis and Questions for the Language Author

This document is based on `SYNTAX.md`, the repository structure, `README.md`, `BUILD_SYSTEM.md`, `IMPROVEMENTS.md`, compiler sources under `src/`, runtime support under `runtime/`, examples, editor tooling, and the test suite as inspected on 2026-05-15.

It is intentionally direct. Drast is promising as a small self-hosted systems-language experiment. It is not currently close to the standard required for aerospace, automotive, defense, medical, or other safety-critical software. The gap is not polish. The gap is semantics, verification, memory safety, compiler architecture, and proof of rejection for bad programs.

## 1. YOUR ANALYSIS

### What Drast Is Today

Drast is a small self-hosted language toolchain. The compiler is written in Drast, parses indentation-sensitive `.drast` source, emits C++17, and builds through a Drast-native `package.txt` workflow that generates an internal xmake project. The runtime is a C++ header, `runtime/drast_runtime.hpp`, providing I/O, strings, filesystem/process helpers, collections helpers, optionals, diagnostics, and platform utilities. The editor story is already nontrivial: the VS Code language server indexes `.drast` files, offers completions and hovers, formats code, and scans some C++ headers for interop assistance.

That is real progress. A self-hosted compiler, package workflow, runtime, examples, tests, and editor support form an actual toolchain, not merely syntax notes. The project also has an unusually healthy sign for an early language: `SYNTAX.md` documents implementation quirks, and `IMPROVEMENTS.md` records concrete C++ code generation failures rather than pretending they do not exist.

But Drast today is best understood as a C++ surface syntax and transpilation experiment with partial semantic tracking. It is not yet a language with a trustworthy independent semantics. The most important fact is that Drast frequently accepts programs and relies on the downstream C++ compiler to reject or define them. For a safety-oriented language, that is the wrong boundary.

The empirical test run matters:

```text
tests/run_tests.sh ./.drast/build/bin/drast /Users/ashwinpaudel/Documents/Drast
143 passed, 2 failed
```

Many fixtures were counted as passing with notes such as `transpile accepted; downstream C++ build failed` or `failed during package build`. That is acceptable for a bootstrap harness if everyone understands what it means. It is not acceptable evidence that the language is sound, mature, or safe. It means the compiler is often functioning as a source-to-source emitter whose own acceptance criterion is weaker than "this is a valid Drast program."

### Syntax

Drast's indentation-sensitive syntax is one of its clearer design choices. It gives the language a compact visual shape closer to Python, Nim, or Swift-like pseudocode than C++. For teaching, scripting, and small embedded examples, that has value. It also reduces delimiter noise in the common case.

The syntax also carries serious ambiguity and cognitive-load risks:

- Function signatures use `name param;Type, ReturnType`, with semicolons as parameter/type separators and default-value separators. This is compact, but it is not self-evident. Semicolon already has strong statement-terminator associations in C-family languages, and Drast uses it for several unrelated roles: parameter types, field payloads, labeled arguments, positional argument markers, and optional declaration markers.
- Calls are implicit: `println 'hello'`, `add 1 2`. This makes simple code pleasant, but it creates a grammar that must guess where calls end. The audit already found semantic breakage around method call argument parsing, such as `counts.set word current + 1` lowering incorrectly. In a safety language, argument boundaries must be boringly obvious to both humans and tools.
- Backtick is overloaded for pointer types, dereference, and generic arguments. This is clever, but cleverness is a liability in safety-critical code. `T```, `` `pointer``, and ``identity`[T]`` are visually light operations that can mean pointer depth, dereference, or template application. The syntax hides too much risk behind a small glyph.
- Single quotes represent both strings and chars, with `s'...'` forcing strings. This saves a keystroke and creates a footgun. A one-character literal changing type based on length is hostile to refactoring, code review, and generated-code predictability.
- Keywords such as `preview`, `fileprivate`, `discard`, and `with` are accepted but mostly informational or skipped. Accepting syntax whose semantics are not enforced is dangerous because it teaches users that declarations mean more than the compiler guarantees.

The syntax is not hopeless. It has a distinctive personality and can become readable. But Drast must decide whether it values compactness over auditability. For spacecraft, automobiles, and defense systems, auditability wins.

### Type System

Drast's current type system is mostly a mapping layer into C++ types:

- `int`, `float`, `double`, `bool`, `char`, `string`, and `usize` map to C++ primitives and `std::string`/`std::size_t`.
- `{T}` maps to `std::vector<T>`.
- `~T` maps to references in type syntax, while expression-level `~x` can produce address-of behavior or a const reference declaration in some assignment contexts.
- `T`` maps to raw pointers.
- `@[T]` maps to `std::shared_ptr<T>`.
- The implementation also contains support for `*[T]`/`std::unique_ptr<T>`, even though the public syntax document emphasizes `@[T]`.
- `maybe T` maps to `std::optional<T>`.
- `tuple` and `map` lower to `std::tuple` and `std::unordered_map`.

This is pragmatic, but it is not yet a type system in the sense required for a safe language. `tests/TODO.md` explicitly records that undeclared identifiers, use-before-declaration, assignment type mismatches, function call type mismatches, wrong argument counts, non-function calls, duplicate declarations, and missing returns are accepted or delegated downstream. Those are not advanced type-system features. Those are table stakes.

Generic constraints after `with` are parsed and skipped. Function overloading is emitted as C++ overloading, but Drast does not resolve overloads itself. Generic inference is delegated to C++ templates where possible. That means the real generic semantics belong to C++, not Drast. A language cannot claim safety or clarity while punting such major decisions to the target compiler.

The fallback to `auto` when expression type is unknown is especially dangerous. `auto` is useful in generated C++ only after the source language has already proven what the program means. In Drast today, it can become a way to avoid knowing.

### Memory Safety

Memory safety is currently the largest design hole.

The examples show the author understands this is unresolved. `Examples/Memory and Controls/memoryManagement.drast` says the goal is for users not to worry about pointers, that the language should be lightweight, and that memory should be managed at compile time, "just like Rust." That is the right aspiration. The implementation is not close to that aspiration.

Today Drast exposes or implies:

- raw address-of and dereference behavior through `~` and backtick;
- raw pointer type syntax through trailing backticks;
- references through `~T`;
- shared ownership through `@[T]`;
- some unique ownership support in the implementation;
- C++ value semantics through structs and fields;
- `std::move` insertion in some last-use cases;
- RAII only insofar as emitted C++ happens to provide it;
- no lifetime checker;
- no borrow checker;
- no formal aliasing rules;
- no mutability model strong enough to prove safety;
- no clear `unsafe` boundary.

That is not memory safety. It is C++ memory behavior with a nicer surface and a small amount of heuristic lowering.

The difference from Rust is decisive. Rust's core contribution is not that it has references or `Box`. It is that ownership, borrowing, lifetimes, mutability, aliasing, moves, drops, and unsafe escape hatches are part of a checked semantic model. Drast currently has surface borrow-like syntax without the proof machinery. That is worse than being explicit C++ in one respect: users may believe the language is protecting them when it is not.

### Error Handling

`maybe T` as `std::optional<T>` is a reasonable starting point. It is simple and familiar from Swift, Rust's `Option`, Zig's optionals, and modern C++.

The dangerous part is the surrounding semantics:

- `optional.value` maps to `.value()`, which may throw.
- `try`/`catch` lowers to C++ exception handling.
- A current narrowing catches `std::bad_optional_access` in optional unwrap cases, but the language model still mixes absence with exception control flow.
- Assignment from optional to non-optional can emit `.value_or(default)`, silently replacing absence with a default value.
- There is no explicit `Result`/error-union model.
- There is no panic strategy, no no-exceptions profile, and no embedded failure policy.

In safety-critical software, silent defaulting is often worse than a crash. A missing sensor value, failed parse, absent command, or invalid state cannot quietly become `0`, `false`, `std::string{}`, or `{}` unless the program explicitly says that default is safe.

Drast needs to decide whether ordinary failure is value-level, exception-level, panic-level, or impossible-by-construction. Today it is all of those at once, inherited from C++.

### Control Flow and Pattern Matching

`if`, `while`, `for`, `match`, `break`, `continue`, and `nothing` form a reasonable control-flow base. The explicit `nothing` placeholder is a good idea for indentation-sensitive syntax because it prevents accidental empty blocks.

But control-flow analysis is not yet trustworthy. Missing returns are known to be accepted. Match exhaustiveness is not established as a general language guarantee. Data enum matching is documented, partially implemented, and also appears in audit notes as an area of invalid C++ lowering. In the current inspected test run, `data_enum_match` failed before building with parser errors. That means data enum pattern matching is not yet a stable semantic feature.

For safety domains, control flow must support proof obligations: all paths return, all enum cases are handled, fallthrough is impossible unless explicit, loop bounds are analyzable where required, and nontermination is either ruled out or consciously accepted.

### Interop Strategy

Drast's C++ interop is a strength and a risk.

The strength is obvious: `use Arduino.h`, `use file native.hpp`, and quoted/unquoted header emission make it easy to use existing C++ and embedded ecosystems. This is a pragmatic route for a new systems language. Carbon is pursuing C++ interop from the other direction, with a much larger design effort. Nim also shows the appeal of compiling to C/C++ and using existing ecosystems.

The risk is that `#include` is not interop semantics. It does not define ABI compatibility, ownership transfer, exception boundaries, lifetime contracts, calling conventions, thread-safety, integer widths, macro behavior, or whether a function is safe to call. Editor header scanning can improve completions, but completions are not a type-checked FFI.

For safety-critical use, every interop boundary must be explicit about trust. Rust has `unsafe extern`; SPARK/Ada has explicit interfacing and analyzability constraints; Zig makes C interop direct but keeps many operations explicit. Drast currently has no comparable boundary. A header can enter the program as if it were ordinary Drast surface area.

### Build System

The `package.txt` workflow is a real asset. It is small, inspectable, and aligned with the language's self-hosting story. The CLI is coherent: `drast init`, `drast build`, `drast run`, target shortcuts, and generated C++ under `.drast/build/generated/<target>/`.

The build system also raises safety and reproducibility concerns:

- The backend forces `clang++` in generated xmake projects.
- The manifest supports shell commands for prebuild, postbuild, and command targets.
- Placeholder substitution is string-based.
- `library` and `embed` target kinds are reserved but not implemented.
- There is no lockfile, dependency integrity model, reproducible build mode, toolchain pinning policy, or cross-compilation story visible in the language surface.
- Generated C++ is marked read-only, which is useful against accidental edits, but not a correctness guarantee.

This is fine for an early toolchain. It is not enough for regulated environments. Safety-critical build systems must be deterministic, traceable, version-pinned, auditable, and hostile to ambient machine state.

### Compiler Architecture

`SYNTAX.md` says parsing and statement body emission are still intertwined. The code confirms the concern: the parser builds AST declarations while function bodies are already emitted as C++ strings. This is the central compiler-architecture problem.

A serious language implementation needs at least these layers:

1. Lexing and parsing into a source AST.
2. Name resolution.
3. Type checking.
4. Borrow/ownership/mutability checking if memory safety is a goal.
5. Control-flow analysis.
6. A typed intermediate representation.
7. Lowering to C++ or another backend.
8. Diagnostics derived from source semantics, not C++ accidents.

Drast currently compresses too many of these into parser/codegen behavior. That is why problems like unknown identifiers, bad calls, generic leakage, implicit-call ambiguity, missing returns, and data enum lowering arise. Without a typed IR, the compiler cannot reliably answer basic questions such as "what does this expression mean?", "can this value be moved?", "does this borrow escape?", or "does this function return on all paths?"

### Runtime and Standard Library

The runtime is useful, but it is broad. It includes I/O, filesystem, process execution, random number generation, diagnostics, string helpers, maps, vectors, optionals, and platform functions. For normal desktop programming, this is convenient.

For embedded and safety-critical domains, it is a liability unless the language defines profiles:

- hosted vs freestanding;
- runtime vs no runtime;
- heap vs no heap;
- exceptions vs no exceptions;
- filesystem/process allowed vs forbidden;
- deterministic random vs nondeterministic random;
- standard library subsets by target class.

`use no_runtime` exists, but the semantics of a no-runtime subset are not yet a language contract. It is currently an include behavior.

### Prior Art

Rust is the relevant benchmark for memory safety. Drast should not imitate Rust syntax blindly, but it must understand that Rust's value is in the checked model: ownership, borrowing, lifetimes, move semantics, `unsafe`, traits, exhaustive pattern matching, `Result`, `Option`, and compiler-enforced aliasing rules. Drast currently has borrow-like syntax without borrow checking, optional-like syntax without a disciplined absence model, and generics without enforced constraints.

Zig is the relevant benchmark for explicit systems programming without a borrow checker. Zig favors visible allocation, explicit error unions, comptime, direct C interop, and a strong "no hidden control flow" culture. Drast is currently less explicit than Zig about allocation, error paths, implicit calls, pointer behavior, and backend consequences.

Ada and SPARK are the relevant benchmark for safety-critical credibility. Their value is not fashion; it is specification, contracts, analyzability, restricted runtime profiles, proof tooling, deterministic subsets, and decades of certification experience. If Drast wants aerospace or automotive credibility, Ada/SPARK is the bar to study, not just Rust.

Carbon is relevant because it is explicitly trying to interoperate with C++ while improving the programming model. The lesson is caution: C++ interop is not a shortcut around language design. It is a massive design problem in its own right.

Swift is relevant for optionals, protocols, value/reference distinctions, ARC, generics, and a clean surface syntax. Swift shows how much semantic machinery sits beneath friendly syntax. Drast currently resembles some Swift surface ideas but lacks the depth behind them.

Nim is relevant because it is indentation-sensitive, pragmatic, expressive, and C/C++-backend-friendly. Nim shows both the attraction and the danger of a friendly high-level surface over a C-like backend: ergonomics do not automatically create safety-critical suitability.

### Bottom Line

Drast has the shape of a serious experiment. It does not yet have the semantics of a serious safety language.

The strongest parts are the self-hosting story, small codebase, direct build workflow, C++ ecosystem access, editor investment, and willingness to audit generated C++. The weakest parts are exactly the parts that matter most for the stated high-end ambitions: type checking, memory safety, error semantics, compiler architecture, interop boundaries, formal specification, and validation.

The next milestone should not be more syntax. It should be making Drast reject invalid programs before C++ sees them, defining the semantic model in writing, and building a typed IR that can support ownership, control-flow, and error analysis. Until then, any safety-critical positioning is premature.

## 2. QUESTIONS FOR THE LANGUAGE AUTHOR

### Mission and Scope

1. What is Drast's actual target: a friendly C++ transpiler, an embedded scripting-like language, a Rust/Zig/Ada competitor, or a safety-critical systems language?
2. Which domains are in scope for the next year: desktop CLI tools, Arduino-class embedded systems, Linux services, automotive firmware, flight software, or something else?
3. What programs should Drast intentionally make impossible?
4. What is the smallest safety claim you want Drast to make, and what evidence will support it?
5. Is C++ intended to remain the permanent semantic backend, or is it a bootstrap target?
6. If Drast and C++ disagree, which language defines the meaning of the program?
7. Will generated C++ be treated as an implementation detail or as a reviewable artifact users are expected to inspect?

### Specification and Semantics

1. Where is the formal grammar?
2. Where is the reference semantics for declarations, expressions, evaluation order, conversions, ownership, and errors?
3. What is Drast's list of undefined behavior?
4. Which behavior is implementation-defined?
5. Which behavior is target-defined?
6. Does Drast inherit C++ undefined behavior, or does it prevent it by construction?
7. What is the rule for evaluation order of implicit call arguments?
8. What is the rule for temporary lifetimes?
9. What is the rule for destruction order?
10. What must every conforming Drast compiler reject?
11. What is the plan for a conformance test suite independent of the current compiler?

### Syntax and Human Factors

1. Why is the semicolon used for so many unrelated roles?
2. Are implicit calls worth the parsing ambiguity and review burden?
3. How will readers know where an implicit call's argument list ends without mentally running the parser?
4. Why should backtick mean pointer depth, generic argument application, and dereference?
5. Is a one-character quoted literal becoming `char` instead of `string` acceptable in a language aimed at reliability?
6. Should Drast require explicit syntax for dangerous operations such as dereference, unwrap, address-of, raw pointer creation, and FFI calls?
7. Will `preview`, `fileprivate`, `discard`, and `with` remain accepted without enforcement?
8. What is the policy for removing syntax that is pleasant but hard to audit?
9. How will the language avoid becoming a collection of clever glyphs rather than a reviewable engineering notation?

### Type System

1. Is Drast nominal, structural, or a hybrid?
2. Are structs nominal types even when fields match?
3. Are protocols structural, nominal, or explicit conformance only?
4. How are overloads resolved in Drast before C++ emission?
5. Will Drast store and check full function signatures?
6. Will wrong arity be a Drast diagnostic rather than a downstream C++ diagnostic?
7. Will argument type mismatch be a Drast diagnostic?
8. Will undeclared identifiers always be rejected before C++ emission?
9. Will use-before-declaration be legal, and if so under what declaration categories?
10. Are local variables definitely assigned before use?
11. Are duplicate declarations in the same scope illegal?
12. What are the exact implicit conversions?
13. Are numeric conversions explicit only?
14. What are the integer widths for `int`, `uint`, and `usize` on every target?
15. What happens on signed overflow?
16. What happens on unsigned overflow?
17. Does Drast have checked arithmetic, wrapping arithmetic, saturating arithmetic, or all three?
18. What is the type of an empty array literal?
19. What is the type of `nil` without contextual typing?
20. Will `auto` be permitted in generated C++ when Drast does not know the source type?
21. How will generic constraints be represented and enforced?
22. What does `with T as Copyable` mean semantically?
23. Are generics monomorphized, type-erased, or delegated to C++ templates?
24. Can generic code depend on operations not present in constraints?
25. How are recursive types represented safely?
26. How are data enum payload types checked?
27. How is match exhaustiveness checked?
28. What is the variance model for generics, references, pointers, optionals, and containers?

### Ownership, Borrowing, and Memory Safety

1. What does ownership mean in Drast?
2. What does borrowing mean in Drast?
3. Is `~T` a mutable borrow, immutable borrow, C++ reference, raw pointer, or abstract borrow?
4. Is expression-level `~x` guaranteed non-null?
5. Can a borrow escape the lifetime of its owner?
6. What static analysis prevents dangling references?
7. What static analysis prevents use-after-move?
8. What static analysis prevents double-free?
9. What static analysis prevents iterator invalidation?
10. What static analysis prevents data races?
11. When are raw pointers allowed?
12. How are nullable pointers represented?
13. Is `T`` a raw pointer, and if so why is it in the safe language?
14. Should raw pointer creation require an `unsafe` block?
15. What is the exact aliasing rule for mutable data?
16. Can there be two mutable aliases to the same object?
17. Can there be mutable aliasing through `shared_ptr`?
18. Does `@[T]` mean shared ownership, heap allocation, reference-counted ownership, or just "put this on the heap"?
19. Why should shared ownership be the default heap spelling?
20. When should Drast emit `unique_ptr` instead of `shared_ptr`?
21. How does Drast prove that converting a heap alias into a reference is safe?
22. How does Drast handle cycles in shared ownership?
23. Are destructors deterministic?
24. Can destructors throw?
25. What are the rules for `deinit`?
26. Can `deinit` access partially moved fields?
27. How does Drast handle self-referential structs?
28. Does Drast permit placement, custom allocators, arenas, or region allocation?
29. Is heap allocation allowed in safety-critical profiles?
30. Is there a no-heap subset?
31. How are stack limits modeled?
32. How are recursive calls controlled or forbidden in safety profiles?

### Optionals, Errors, and Failure

1. Is `maybe T` Drast's only recoverable failure mechanism?
2. Will Drast add a `Result` or error-union type?
3. Is absence an error, a value, or both?
4. Should `.value` be allowed in safe code if it can throw?
5. Should forced unwrap require explicit syntax that is visually dangerous?
6. Should optional-to-non-optional assignment ever silently emit `.value_or(default)`?
7. Who chooses the default for absent optionals?
8. What is the panic strategy?
9. Is there a no-panic subset?
10. Are C++ exceptions allowed in normal Drast code?
11. Are C++ exceptions allowed in embedded profiles?
12. Can exceptions cross Drast/C++ FFI boundaries?
13. Are destructors run during error unwinding?
14. How are fatal errors reported on targets without stderr, filesystem, or OS process support?
15. How does Drast distinguish programmer error, recoverable runtime failure, and hardware/environment failure?

### Control Flow, Data Enums, and Pattern Matching

1. Will every non-void function require proven return coverage?
2. Will loops have analyzable bounds in safety profiles?
3. Will unbounded `while true` require annotation?
4. Are `break` and `continue` permitted in analyzable safety subsets?
5. Is `match` required to be exhaustive for enums and data enums?
6. What happens when a `match` is not exhaustive?
7. Are value matches lowered to chains or switches?
8. Are data enum matches guaranteed to bind payloads by reference, copy, or move?
9. Can match arms mutate payloads?
10. How are recursive data enums represented without unsafe ownership tricks?
11. How are payload field names resolved when they collide with locals?
12. Is `default` discouraged when matching closed enums?

### Protocols, Traits, Interfaces, and Dispatch

1. What is a protocol semantically?
2. Is protocol conformance checked or only recorded?
3. Are protocol methods statically dispatched, dynamically dispatched, or both?
4. When does a protocol imply a C++ vtable?
5. Are protocol methods allowed to be mutating?
6. How is constness represented in protocol requirements?
7. How is `override` enforced?
8. What happens if a type claims conformance but misses a method?
9. Can protocols express associated types?
10. Can protocols express generic constraints?
11. Can protocols express ownership requirements such as copyable, movable, sendable, or nonescaping?

### Closures and Higher-Order Code

1. Will Drast have closures?
2. If yes, what is the capture syntax?
3. Are captures by value, by immutable borrow, by mutable borrow, or inferred?
4. How are closure lifetimes checked?
5. Can closures escape their defining scope?
6. Can closures capture move-only values?
7. Can closures be generic?
8. How do closures interact with C++ lambdas and function pointers?
9. Are closures allowed in safety-critical profiles?
10. How will closure capture avoid hidden allocation?

### Concurrency, Real-Time, and Embedded Use

1. Does Drast have a concurrency model?
2. Are threads part of the standard library?
3. Are async tasks planned?
4. What is the memory model?
5. Are atomics exposed?
6. Are locks exposed?
7. Can the type system express `Send`, `Sync`, ownership transfer, or thread confinement?
8. How are data races prevented?
9. How are interrupt handlers represented?
10. Can code be marked interrupt-safe?
11. Can code be marked allocation-free?
12. Can code be marked nonblocking?
13. Can code be marked bounded-time?
14. What is the story for hard real-time systems?
15. What standard library APIs are forbidden in hard real-time profiles?
16. How will Drast support targets without filesystem, process execution, exceptions, RTTI, or dynamic allocation?

### C++ Interop and Unsafe Boundaries

1. Is `use header.hpp` safe by default?
2. How does Drast know the types declared in a C++ header?
3. Is header scanning part of the compiler or only the editor?
4. How are C++ templates imported?
5. How are overloaded C++ functions imported?
6. How are macros handled?
7. How are namespaces handled?
8. How are C++ references, pointers, smart pointers, and ownership conventions represented in Drast?
9. How is ownership transfer across FFI declared?
10. How are borrowed parameters from C++ annotated?
11. How are nullability and lifetime of C++ pointers described?
12. How are C++ exceptions handled across the boundary?
13. How is ABI compatibility checked?
14. How is name mangling controlled?
15. How are compiler flags, standard-library variants, and target ABI differences captured?
16. Should all C++ interop require an explicit `unsafe` declaration?
17. How will Drast audit calls into Arduino or vendor HAL headers?
18. Can unsafe wrappers be reviewed and then exposed as safe Drast APIs?
19. What are the criteria for such a wrapper to be considered safe?

### Build, Packaging, and Reproducibility

1. Is `package.txt` intended to remain line-oriented, or will it become a structured manifest with a schema?
2. How are dependencies declared?
3. How are dependency versions pinned?
4. How is dependency integrity verified?
5. Is there a lockfile?
6. Are builds reproducible?
7. Is the exact C++ compiler version recorded?
8. Is xmake version pinned?
9. Why does the backend force `clang++`?
10. How will cross-compilation work?
11. How will embedded targets be specified?
12. How will target-specific runtime profiles be selected?
13. Are shell prebuild/postbuild commands acceptable in regulated builds?
14. How are generated files traced back to source?
15. Can every binary be traced to exact Drast source, compiler binary, runtime header, C++ compiler, flags, and dependencies?
16. What is the bootstrap trust model?
17. How is the legacy compiler seed audited?
18. How will reproducible self-hosting be demonstrated?

### Compiler Architecture

1. When will parser and C++ emission be separated?
2. What will the typed IR look like?
3. Where will name resolution live?
4. Where will type checking live?
5. Where will borrow/ownership checking live?
6. Where will control-flow analysis live?
7. How will diagnostics point to source constructs rather than generated C++ failures?
8. Will the compiler maintain symbol tables for functions, methods, globals, locals, types, protocols, enum variants, and imports?
9. Will the compiler track value category: lvalue, rvalue, moved, borrowed, mutable, const?
10. Will the compiler track effect information such as throws, allocates, blocks, reads global state, writes global state, or calls unsafe code?
11. Will there be incremental compilation?
12. Will there be a stable serialized module format?
13. How will the compiler handle multiple source files without C++ ODR accidents?
14. How will generated C++ be snapshot-tested?
15. Will generated C++ be compiled with `-Wall -Wextra -Werror`?
16. Will Drast run sanitizers in CI?
17. Will Drast fuzz the lexer, parser, type checker, and code generator?

### Tooling and Diagnostics

1. Which diagnostics must come from the compiler rather than the editor?
2. Can the editor ever accept code the compiler rejects, or reject code the compiler accepts?
3. How are diagnostics tested?
4. Are fixes and suggestions part of the compiler contract?
5. Does `discard` suppress a real warning, and if not, when will it?
6. Will Drast have a formatter with a stable style contract?
7. Will Drast have lint profiles for safety-critical code?
8. Will Drast support documentation generation?
9. Will Drast support coverage, profiling, and static-analysis reports tied back to source?

### Testing, Verification, and Certification

1. What is the minimum test evidence required before calling a feature implemented?
2. Should a test count as passed if Drast accepts the source but generated C++ fails to build?
3. Should a negative test count as passed if the C++ compiler rejects what Drast accepted?
4. What is the plan to convert known pending diagnostics into real compiler errors?
5. Will every language feature have parser, type-checker, codegen, runtime, and negative tests?
6. Will there be property tests for parser round-tripping and pretty-printing?
7. Will there be differential tests against expected C++ behavior?
8. Will there be memory-safety tests under ASan, UBSan, TSan, and MSan?
9. Will there be MISRA C++ or AUTOSAR C++ checks on generated code?
10. Will there be a SPARK/Ada comparison matrix for safety claims?
11. What is the tool qualification path if Drast is ever used in DO-178C, ISO 26262, IEC 61508, or similar contexts?
12. Who signs off on language changes that affect safety semantics?
13. How are regressions in generated C++ reviewed?
14. How long will old compiler versions be supported?

### Standard Library and Runtime Profiles

1. What belongs in the core language versus the runtime?
2. Is `use std` an import, a profile switch, or both?
3. What exactly does `use no_runtime` guarantee?
4. Can a no-runtime program still use heap allocation?
5. Can a no-runtime program still use exceptions?
6. Can a no-runtime program still use RTTI?
7. Which APIs are available on bare metal?
8. Which APIs are deterministic?
9. Which APIs allocate?
10. Which APIs can fail?
11. How are allocation failures represented?
12. How are filesystem and process APIs excluded from embedded targets?
13. How are random numbers seeded and made deterministic when required?
14. Is the runtime header part of the language specification?

### Governance and Long-Term Design

1. Who decides when syntax is removed?
2. What compatibility promise will Drast make?
3. Will there be editions or language versions?
4. How will experimental features be marked?
5. How will unsafe or incomplete features be prevented from entering safety profiles?
6. What is the contribution standard for features touching semantics?
7. Will every feature require a design note, semantics note, tests, and generated-code review?
8. What is the process for rejecting attractive syntax that weakens the language?
9. What must be true before Drast can honestly claim memory safety?
10. What must be true before Drast can honestly claim suitability for embedded systems?
11. What must be true before Drast can honestly claim suitability for automotive software?
12. What must be true before Drast can honestly claim suitability for spacecraft?

## Final Assessment

Drast should continue, but it should stop measuring progress primarily by new syntax accepted and start measuring progress by invalid programs rejected, semantics written down, generated C++ made boring, and dangerous operations made explicit.

The language has enough shape to be worth taking seriously. That is why the questions are hard. A toy language does not need a lifetime story, FFI boundary model, deterministic build profile, or certification path. A language that wants to live near flight software, cars, defense systems, or other consequential machines absolutely does.

