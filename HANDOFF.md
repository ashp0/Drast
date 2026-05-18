## Context

You are working on **Drast**, a self-hosted programming language authored by Ashwin Paudel (solo). The project lives at `/Users/ashwinpaudel/Documents/Drast`. The compiler is currently self-hosted (the legacy C++ implementation was recently retired) and emits C++ as its backend via `xmake` and `clang++`. LLVM is the eventual real backend; the C++ emission is a bootstrap detail.

Ashwin's long-term ambition is for Drast to be usable for safety-critical systems (aerospace, surgical, defense) while also being approachable for hobby/general-purpose use. The honest gap between the current state and that goal is 10–20 years, but the *next* year of work is what closes the first chunk of it.

In a recent strategic session, Ashwin and a previous agent jointly authored `SEMANTICS.md` — a real, rigorous language specification covering memory model, mutability, type discipline, error handling, absence (optionals), integers, floats, booleans, characters, and strings. **`SEMANTICS.md` is the source of truth for what Drast should accept and reject.** It is the contract you are implementing against.

Your job is to make the existing Drast compiler enforce the rules in `SEMANTICS.md`, build a conformance test suite that proves it, and surface places where the current source contradicts the spec. You are not adding new syntax. You are not replacing the C++ backend. You are not redesigning the language. You are making the compiler reject the programs it should reject and emit clear diagnostics when it does.

## Read first, in this order

1. `MEMORY.md` (in `/Users/ashwinpaudel/.claude/projects/-Users-ashwinpaudel-Documents-Drast/memory/`) — Ashwin's context and how to work with him.
2. `SEMANTICS.md` — the language spec. Every rule has an "Accepts," a "Rejects," and possibly "Open questions." The Rejects are diagnostics you must implement; the diagnostic codes (E0001, E0010, etc.) are the canonical IDs.
3. `SYNTAX.md` — the existing informal grammar reference. Trust `SEMANTICS.md` when the two disagree; flag the disagreement.
4. `src/` and `tests/` — current compiler.

## Your job, in priority order

### Phase 1 — Make the compiler enforce the easy half of SEMANTICS.md

The following rules from `SEMANTICS.md` are mechanically straightforward to enforce. They do **not** require a borrow checker. Implement them, with a positive test and a negative test for each.

1. **§3.1 / §3.2 — Integer types and overflow.**
   - Remove `int` and `uint` as keywords.
   - Integer literals get the type required by context; default to `i32` when ambiguous.
   - Literal overflow at compile time (`x: u8 := 300`) — emit `E0050`.
   - No implicit cross-width conversions; require `as`. Emit `E0051`.
   - No implicit signed/unsigned conversions. Emit `E0052`.
   - In debug builds (and always in `#profile(embedded|safety_critical)`), arithmetic ops emit overflow checks. In release builds (default profile), they wrap.
   - Implement `&+`, `&-`, `&*`, `&<<` (wrapping) and `|+|`, `|-|`, `|*|` (saturating) operators.
   - Division/modulo by zero always panics regardless of profile.

2. **§1.2 — Mutability.**
   - Bindings are immutable by default; reassigning an immutable binding is `E0010`.
   - `mut x = ...` enables reassignment.
   - Function parameters are immutable inside the function body unless declared `mut`.

3. **§1.3 — Type discipline.**
   - No `any` type. If one currently exists in the implementation, retire it.
   - Function parameters, return types, struct/enum fields, and globals require explicit type annotations — `E0021`, `E0022`.
   - Local bindings infer from the right-hand side.
   - Heterogeneous array literals — `E0020`.

4. **§1.5 — `maybe T` and the absence model.**
   - Implement `maybe T` as a sum type with `Some[T]` and `None` constructors. It may be stdlib-defined with compiler-known semantics for niche optimizations.
   - Treating a `maybe T` as a `T` (field access, function pass) — `E0040`, `E0041`.
   - Implement the unwrap surface: `match`, `if let`, `.value_or[default]`, `->force[]`, `->force_with[msg]`.
   - There is no `null` keyword in safe code.

5. **§1.4 — `Result[Ok, Err]` and the `try` operator.**
   - Implement `Result` as a sum type with `Ok[T]` and `Err[E]` constructors.
   - The `try` operator (prefix `try expr`) returns inner `Ok` or early-returns `Err` to the caller. Requires the enclosing function's return type to be `Result[_, E']` or `maybe _` (where `E` converts to `E'`).
   - There is no `throw` / `catch` in Drast. If the grammar accepts those tokens, emit `E0030`.

6. **§3.5 — Booleans.**
   - `if`, `while`, ternary require `bool`. Non-`bool` condition is `E0080` / `E0081`.
   - `=` is not an expression — `E0082`.
   - No truthiness coercion for any type.

7. **§3.6 — Char and string.**
   - `'...'` is always `string` (or `~string` if in a static context).
   - `c'x'` is `char` (Unicode scalar). Multi-scalar `c'...'` — `E0091`.
   - Indexing `string` with an integer — `E0090`.
   - String mutation on an immutable binding — `E0092`.
   - Strings are UTF-8 internally. Provide `.bytes[]`, `.chars[]`, `.byte_at[i]`, `.slice[start..end]`.

8. **§3.4 — Floats.**
   - Two types: `f32`, `f64`. Literal default: `f64`. No `float` keyword. Literal mismatches — `E0070`.
   - No implicit int↔float conversion — `E0071`.
   - IEEE 754 semantics; NaN comparisons follow the standard.

### Phase 2 — Profiles, newtypes, and the compiler-as-spec link

9. **§3.3 — Newtypes.**
   - Implement `type Name = T` as a distinct nominal type.
   - Construction: `Name[value]`. Projection: `name.inner` or `name as T`.
   - Mixing `Name` with `T` or sibling newtypes — `E0060`, `E0061`.
   - No automatic operator forwarding.

10. **§1.4, §3.2 — Profile attributes.**
    - Implement `#profile(...)` as a function/module attribute with values: `default`, `embedded`, `safety_critical`.
    - In `embedded` and `safety_critical`, force overflow checks always (override release-build wrap).
    - In `embedded` and `safety_critical`, panic strategy is `abort` (no unwinding). Default profile uses `unwind`.
    - In `safety_critical`, emit warnings when an `f32`/`f64` reaches a comparison, FFI call, or conversion without an `is_finite` guard.

11. **`tests/conformance/` — the conformance test suite.**
    - For every error code mentioned in `SEMANTICS.md` (E0001..E0092 and beyond), create at least one positive test (`.must-accept.drast`) and one negative test (`.must-reject.drast`) under `tests/conformance/`.
    - Negative tests assert on the diagnostic code, not the exact wording.
    - Make `xmake test` (or whatever the project runner is) run the conformance suite and fail on any deviation.
    - Tests run independently of the C++ backend success — if Drast accepts the source, the test passes whether or not C++ subsequently compiles. We are testing Drast, not Clang.

### Phase 3 — Deferred (do NOT attempt in this session unless explicitly asked)

These are mentioned in `SEMANTICS.md` and are real follow-up work, but each is large enough to warrant a separate, dedicated effort:

- **Full borrow checker.** Enforcing §1.1 properly is months of work involving lifetime inference and a dataflow analysis. For Phase 1, treat borrows as untyped pointers if needed and add a `TODO(borrow-check)` comment at each call site that *would* be checked. Do not pretend to enforce ownership/aliasing rules that you have not actually implemented.
- **Range-refined types** (`type Altitude = i32 in 0..60_000`).
- **Concurrency model** — `SEMANTICS.md` has no section for this yet; do not invent one.
- **Standard library design** beyond what §3.6 mentions for strings.

## What you must NOT do

- **Do not add new syntax.** If you find yourself wanting to, stop, write the proposal in `SEMANTICS.md` under "Open questions," and ask Ashwin.
- **Do not silently relax a rejection.** If `SEMANTICS.md` says the compiler must reject something and the current source accepts it, the fix is to add the rejection — not to weaken the spec.
- **Do not modify `SEMANTICS.md` unilaterally.** You may add to "Open questions" sections. Changes to a "Rule." or "Rejects." section require Ashwin's explicit approval, captured in the commit message.
- **Do not pretend a feature works when it doesn't.** Half-implementing the borrow checker, for example, would create false safety claims. If a check is not implemented, the diagnostic should say so explicitly (`E0001 not yet enforced — see HANDOFF.md`).
- **Do not commit without running the conformance suite.** Every commit must pass `xmake test` (or the equivalent).
- **Do not "improve" tests that currently fail.** A failing test that matches the spec is doing its job. The fix is the compiler, not the test.

## How to work with Ashwin

- Ashwin is the solo author. He has strong taste but limited formal background in PL theory. When you use a term he might not know (variance, ABI, monomorphization, dataflow analysis), give a one-line plain-English intuition before going deeper.
- He explicitly asked for direct, honest feedback rather than encouragement. If a plan in `SEMANTICS.md` is impractical or a current design has a flaw, say so plainly with the reasoning.
- He values incremental, reviewable progress over big-bang refactors. Land one rule at a time with its test.

## How to commit

Commit per rule, not per phase. A good commit looks like:

```
enforce E0010: cannot assign to immutable binding

- Adds the `mut` keyword check in the type checker.
- Adds tests/conformance/E0010.must-reject.drast and E0010.must-accept.drast.
- Updates SEMANTICS.md if the diagnostic message refines the prior text.
```

If a commit closes an "Open questions" item in `SEMANTICS.md`, edit that section to record the resolution.

## When you finish

When all Phase 1 items are landed and the conformance suite is green:

1. Update `SEMANTICS.md`'s status line to note Phase 1 enforced.
2. Write a short `PROGRESS.md` listing every error code enforced, every rule still pending (especially the deferred Phase 3 items), and any open questions raised during implementation.
3. Tell Ashwin which rules from `SEMANTICS.md` you were unable to enforce and why.

Then stop. Do not start Phase 2 without Ashwin's go-ahead.

## Starting

Begin by reading `SEMANTICS.md` in full, then `src/` to understand the current compiler structure. Make a plan for Phase 1, share it with Ashwin, and only start coding once he confirms.

Good luck. The goal is not to finish — it is to make every accepted program well-defined and every rejected program well-rejected.
