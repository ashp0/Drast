# Drast Language Semantics

**Status:** Draft. Authored 2026-05-15.
**Purpose:** This document defines what every Drast construct *means*. `SYNTAX.md` defines what programs *look like*; this document defines what they *do* and — equally important — what the compiler **must reject**.

> A language is its rejections. Anything a compiler accepts becomes part of the language whether you meant it to or not.

---

## 0. Reading guide

Each section follows this template:

- **Rule.** A short statement of the rule, in plain English.
- **Rationale.** Why this rule exists.
- **Accepts.** Examples the compiler must accept.
- **Rejects.** Examples the compiler must reject, with the exact diagnostic.
- **Open questions.** Things still undecided. Resolve before shipping.

If a section has open questions, the feature is **not specified** and should be treated as experimental.

---

## 1. Foundations

### 1.1 Memory model

**Rule.** Drast uses **ownership with borrowing**, in the style of Rust.

- Every value has exactly **one owner** at any time.
- When the owner's lexical scope ends, the value is **destroyed deterministically**: its destructor runs, then its memory is reclaimed. No garbage collector. No runtime allocator tracking.
- Ownership can be **transferred** (moved) from one binding to another. After a move, the source binding is **dead**: any use is a compile error.
- A value may be **borrowed** to allow temporary access without transferring ownership. Borrows are checked at compile time. A borrow may not outlive its owner.
- There is **no implicit copying** of owned heap values. Trivially-copyable values (integers, floats, bools, raw pointers in `unsafe`) are exempt and follow value semantics.

**Rationale.** Determinism and zero runtime cost are non-negotiable for embedded and safety-critical use. A garbage collector forbids us from aerospace certification (DO-178C). Reference counting pays cycles tax and leaks on graphs. Ownership gives memory safety *by construction* at compile time, with zero runtime overhead, while still allowing escape hatches via `unsafe`.

**Accepts.**

```drast
main
    s = 'hello'      // s owns the string
    print_it[~s]     // borrow: print_it reads s, does not own it
    take_it[s]       // move: ownership transferred into take_it
                     // (nothing to free here; take_it freed it)

print_it s;~string \n print[s]   // borrows, does not own
take_it s;string \n           // takes ownership, frees at scope end
```

**Rejects.**

```drast
main
    s = 'hello'
    take_it [s]       // move
    print_it[~s]     // ERROR E0001: use of moved value 's'
                     //              note: ownership moved into 'take_it' on line above
```

```drast
dangling, ~string
    s = 'hello'
    return ~s        // ERROR E0002: returned borrow outlives its owner
                     //              note: 's' is destroyed when this function returns
```

**Open questions.**
- Exact syntax for borrows. Currently `~` doubles as borrow and as C++ `&`. We must distinguish *read-only borrow* (`~T`) from *mutable borrow* (something else, TBD) before this is shippable.
- Whether `@[T]` (shared ownership, refcounted) exists at all, or is only available in `unsafe` / opt-in. Will resolve when we cover shared ownership.
- Whether destructors can fail, and if so, what happens. Defer to error model section.

### 1.2 Mutability default

**Rule.** All bindings are **immutable by default**. Mutability is opt-in via the `mut` keyword.

- `x = expr` declares an immutable binding. Reassignment is a compile error.
- `mut x = expr` declares a mutable binding. Reassignment is allowed.
- Borrows distinguish *immutable* from *mutable*:
    - `~T` is a **shared (immutable) borrow.** Many may exist simultaneously. Holders may read but not modify.
    - `~mut T` is an **exclusive (mutable) borrow.** Only one may exist at a time, and no shared borrows may coexist with it.
- **The aliasing-XOR-mutability rule.** At any program point, a value is either borrowed mutably by exactly one party, or borrowed immutably by any number of parties. The compiler enforces this statically.

**Rationale.** Immutable-by-default surfaces every mutation in the source, making review and audit tractable — a core requirement for safety-critical software. The aliasing rule eliminates iterator invalidation, accidental aliasing bugs, and data races at compile time. Pairs naturally with ownership: borrows are the mechanism by which the borrow checker enforces this rule.

**Accepts.**

```drast
main
    x = 5                  // immutable
    print[x]                // OK, reads only

    mut counter = 0
    counter = counter + 1   // OK, mutable
    counter = counter + 1

    mut list = [1, 2, 3]
    read[~list]             // shared borrow: OK
    read[~list]             // multiple shared borrows: OK
    bump[~mut list]         // exclusive borrow: OK (no shared borrows alive)


read v;~int   \n\t  print[v]
bump v;~mut, int \n\t v = v + 1
```

// Note: The syntax below is just an example, not actual Drast but similar in concept.
**Rejects.**

```drast
fn main [
    x := 5
    x = 6              // ERROR E0010: cannot assign to immutable binding 'x'
                       //              note: declare as 'mut x' to allow reassignment
]

fn main [
    mut list := [1, 2, 3]
    r := ~list                    // shared borrow alive
    bump[~mut list]               // ERROR E0011: cannot borrow 'list' as mutable
                                  //              while shared borrow 'r' is alive
    print[r]
]

fn main [
    mut list := [1, 2, 3]
    a := ~mut list
    b := ~mut list                // ERROR E0012: cannot create second mutable borrow
    bump[a]; bump[b]
]
```

**Open questions.**
- Whether `mut` propagates to struct fields automatically, or whether fields need their own `mut` annotation.
- Interior mutability (a `Cell`-like construct allowing controlled mutation through a shared borrow). Likely needed eventually, definitely `unsafe`-only at first.
- Whether function parameters default to immutable bindings inside the function body (Rust: yes; almost always wanted).

### 1.3 Type discipline

**Rule.** Drast is **fully statically typed**. Every expression has a single type known at compile time. There is no `any` type, no dynamic typing escape hatch, and no implicit runtime type coercion.

- **Inference.** Local bindings, lambda parameters whose context disambiguates, and inferred-return functions do not require explicit annotations. The compiler infers the most specific type from the right-hand side.
- **Annotations required at boundaries.** Function parameters, function return types, struct/enum/protocol fields, and module-level (global) declarations must have explicit type annotations. Rationale: these are the API surface across compilation units; they must not depend on inference of bodies the user may not see.
- **No implicit conversions** except those listed in §3 (Types). In particular: no implicit narrowing, no implicit signed/unsigned conversion, no implicit numeric-to-bool, no implicit nil-to-default.

**Rationale.** Static typing is required for safety-critical certification (DO-178C, MISRA C++, IEC 61508). Strong inference keeps the source visually light — matching the Python-like ergonomics goal — without sacrificing safety. Forbidding `any` prevents the well-documented decay of gradual type systems (TypeScript, mypy) where dynamic values leak through codebases until guarantees become meaningless.

**Accepts.**

```drast
fn add[a: int, b: int] -> int [    // annotations required on parameters and return
    sum := a + b                   // inferred: int
    return sum
]

fn main [
    x := 5                         // inferred: int
    name := 'Ashwin'               // inferred: string
    list := [1, 2, 3]              // inferred: array[int]
    pi := 3.14                     // inferred: float
]
```

**Rejects.**

```drast
mixed := [1, 'hello']              // ERROR E0020: array elements must have a common type
                                   //              found: int, string

fn f[x] -> int [ return x ]        // ERROR E0021: parameter 'x' requires a type annotation

global counter := 0                // ERROR E0022: module-level declaration requires explicit type
                                   //              try: 'global counter: int = 0'

x: int := 5
y: string := x                     // ERROR E0023: cannot implicitly convert 'int' to 'string'
```

**Open questions.**
- Whether return-type inference is allowed for non-exported (file-private) functions to reduce annotation burden.
- How explicit `void` return annotations interact with the current syntax where omitting the comma means `void`; the semantic checker currently treats omission as legacy-explicit `void` rather than rejecting every void function.
- Whether literals with no context (e.g., `42`) default to `i32`, `i64`, or `int`, and the rules for numeric literal type inference.
- Coercion rules between numeric types — fully resolved in §3.

### 1.4 Error model

**Rule.** Drast distinguishes three kinds of failure, with three different mechanisms:

1. **Absence** — handled in §1.5 (optional types). Not an error.
2. **Recoverable failure** — returned as `Result[Ok, Err]`. The `try` operator (alias: `?`) unwraps on `Ok` and early-returns the `Err` to the caller. The only ways to obtain the inner value are `match`, `try`, or explicit `.unwrap_or[default]`.
3. **Programmer error / impossible state** — triggers a **panic**. Includes: array-out-of-bounds, integer overflow (in checked modes), assertion failure, exhaustive `match` reaching an impossible arm, `unwrap` on an `Err` or `None`. Panics are not for control flow.

**Exceptions do not exist in Drast.** No `throw`. No `try`/`catch`. C++ exceptions raised across the FFI boundary are caught by the runtime and converted to an abort (see §8).

**Panic policy is profile-controlled.**
- Default profile: `panic = unwind`. The stack is unwound, destructors run, the process exits. A top-level `catch_unwind[fn]` is available for graceful-shutdown patterns. Panics cannot cross FFI.
- `#profile(embedded)` and `#profile(safety_critical)`: `panic = abort`. The runtime calls a target-defined abort handler immediately. No unwinding, no destructors. Required for bounded-time guarantees and certification.

**Rationale.** Result-based errors are values, not control flow — they are auditable, bounded-time, and visible at every call site. The `try` operator gives the ergonomic single-character version of "if Err return Err." Separating panics from recoverable errors prevents the canonical mistake of using exceptions for both expected failures (file not found) and bugs (null deref). Profile-controlled panic policy is exactly what Rust does and what aerospace requires.

**Accepts.**

```drast
fn read_config[] -> Result[Config, ReadError] [
    text := try read_file['cfg.toml']   // on Err, early-return it; on Ok, bind text
    cfg  := try parse_config[text]
    return Ok[cfg]
]

fn main [
    match read_config[] [
        Ok[c]:  use_config[c]
        Err[e]: print['failed: $e']
    ]
]
```

**Rejects.**

```drast
fn read_config[] throws Config [ ... ]     // ERROR E0030: 'throws' is not a Drast keyword
                                           //              use 'Result[T, E]' for recoverable errors

fn main [
    r := read_config[]            // r: Result[Config, ReadError]
    use_config[r]                 // ERROR E0031: cannot pass Result[Config, ReadError]
                                  //              where Config is expected; use 'try' or 'match'
]
```

**Open questions.**
- Exact spelling of the `try` operator: prefix `try expr`, postfix `expr?`, or both. Postfix `?` is concise but a one-character glyph; prefix `try` is louder and easier to grep.
- Whether `Result` is built-in or a stdlib type with compiler-known semantics (the latter is more flexible but slightly less ergonomic for FFI).
- Error-type composition: how to express "this function returns either FileError or ParseError." Rust uses traits; we will likely use a built-in error union or a `From` protocol.

### 1.5 Absence model (null / optional)

**Rule.** Drast has **no null** in safe code. Absence is expressed by the type **`maybe T`**.

- A binding of type `T` is guaranteed to denote a valid `T`. Dereferencing or accessing fields of a `T` cannot null-fault.
- `maybe T` is a distinct type whose value is either `Some[T]` or `None`. The compiler treats `maybe T` as a sum type, not as "T plus a null bit." In particular, `maybe maybe T` is a distinct type from `maybe T`.
- The inner value of a `maybe T` is obtainable **only** through:
    1. `match` against `Some[x]` / `None`,
    2. `if let x := opt [ ... ]` binding,
    3. The `try` operator inside a function whose return type is `maybe U` or `Result[U, _]`,
    4. `opt->force[]` — forced unwrap. Panics if `None`. Use is discouraged outside `unsafe` and tests.
    5. `opt->force_with[msg]` — same, with a custom panic message.
    6. `opt.value_or[default]` — explicit default, never panics.

**Rationale.** Null is the single most expensive design mistake in programming language history. Making absence a distinct type, with the compiler forcing a check before access, eliminates the entire null-dereference bug class. Choosing the spelling `maybe T` over `T?` keeps the language self-explanatory for newcomers; the cost is mild unfamiliarity for Rust/Swift transplants. Forced unwrap exists because there are situations where the programmer knows more than the type system, but the spelling `->force[]` is deliberately visually loud and easy to grep during audit.

**Accepts.**

```drast
fn find_user[id: int] -> maybe User [ ... ]

fn main [
    u := find_user[42]                  // u: maybe User

    match u [                           // safe handling
        Some[user]: print[user.name]
        None:       print['not found']
    ]

    if let user := find_user[7] [       // 'if let' binding
        print[user.name]
    ]

    name := u.value_or['anonymous']     // safe default
]

fn first_admin[users: ~array[User]] -> maybe User [
    for u in users [
        if u.is_admin [ return Some[u] ]
    ]
    return None
]
```

**Rejects.**

```drast
fn main [
    u := find_user[42]
    print[u.name]                       // ERROR E0040: type 'maybe User' has no field 'name'
                                        //              note: use 'match', 'if let', or '->force[]'

    fn takes_user[u: User] [ ... ]
    takes_user[u]                       // ERROR E0041: cannot pass 'maybe User' where 'User' expected
]
```

**Open questions.**
- Whether `maybe T` is a built-in or stdlib-defined sum type with compiler-known semantics. Recommended: stdlib type with the compiler recognizing it for niche optimizations (e.g., `maybe ~T` should be one pointer wide, with `None` represented as null at the ABI level only — invisible to safe code).
- The exact spelling of the propagation operator: `try expr` keyword vs `expr?` postfix. Decide alongside Result in §1.4.
- Whether `Some` and `None` are reserved names or constructors of the stdlib `maybe` type.

---

## 2. Lexical structure
*To be decided.*

## 3. Types

### 3.1 Integer types

**Rule.** Drast's integer types are exactly: `i8`, `i16`, `i32`, `i64`, `u8`, `u16`, `u32`, `u64`, plus `isize` and `usize` (pointer-width signed/unsigned, used for sizes and indices). **There is no `int` or `uint` keyword.** Every integer binding has a known fixed width on every target.

- **Literal inference.** An untyped integer literal (e.g., `5`) is inferred from context. If no context constrains it, the default is `i32`. Range-overflowing a literal at compile time is a compile error: `let x: u8 = 300` is rejected at parse/check time, not at runtime.
- **No implicit cross-width conversion.** Every conversion between integer types requires `as`: `let y: i64 = x as i64`. The `as` cast panics if the value does not fit in the target type (consistent with the overflow rule below).
- **No implicit signed/unsigned conversion.** `i32 -> u32` and `u32 -> i32` require `as`, and panic on out-of-range values.

**Rationale.** Explicit-width integer types are required for any binary-compatible, multi-target, or auditable system. The Ariane 5 explosion is the canonical case study against implicit conversions. Eliminating `int` / `uint` removes a class of cross-target bugs entirely; the cost is a few extra characters per declaration.

### 3.2 Integer overflow

**Rule.** Integer overflow behavior is **profile-controlled**:

| Profile / build | Overflow on `+ - * <<` | Notes |
|---|---|---|
| `debug` (default) | **Panic** | Bounded-time abort or unwind per §1.4. |
| `release` | **Two's-complement wrap** | Fast release builds. Acceptable for hobby/desktop. |
| `#profile(embedded)` | **Panic (abort)** | Always checked, regardless of debug/release. Required for bounded-time real-time. |
| `#profile(safety_critical)` | **Panic (abort)** | Always checked, regardless of debug/release. Required for certification. |

Explicit operators bypass the check, in every profile:

- `&+`, `&-`, `&*`, `&<<` — **two's-complement wrapping.** Result is well-defined; programmer asserts the wrap is intentional.
- `|+|`, `|-|`, `|*|` — **saturating.** Result clamps to type bounds (e.g., `i8::MAX |+| 1 == i8::MAX`).
- `checked_add[a, b]` / `checked_sub[a, b]` / `checked_mul[a, b]` — return `maybe T`; `None` on overflow.

Division by zero **always** panics, regardless of profile. There is no "wrap-on-divide-by-zero." Modulo by zero likewise panics.

**Rationale.** Release-build wrap matches Rust and gives competitive performance for non-safety code; safety profiles eliminate the dev/prod divergence that this otherwise creates. Explicit `&+` / `|+|` / `checked_` operators surface intentional wrapping/saturating/checking at the call site, which is exactly the discipline that aerospace audit requires. The release-default `wrap` is *not* the safety story — the profile override is.

**Accepts.**

```drast
fn main [
    a: i32 := 2_147_483_647

    // Default arithmetic
    b := a + 1        // panic in debug & in safety profiles; wraps in release

    // Explicit wrap
    c := a &+ 1       // c = -2_147_483_648 in every profile

    // Explicit saturate
    d := a |+| 1      // d =  2_147_483_647 in every profile

    // Checked
    e := checked_add[a, 1]   // e: maybe i32 == None
]

#profile(safety_critical)
fn flight_calc[alt: i32, climb: i32] -> i32 [
    return alt + climb      // overflow panics (aborts) in this profile
]
```

**Rejects.**

```drast
fn main [
    x: u8 := 300              // ERROR E0050: literal 300 does not fit in u8 (max 255)

    a: i32 := 5
    b: i64 := a               // ERROR E0051: no implicit conversion from i32 to i64
                              //              try: 'a as i64'

    s: i32 := -1
    u: u32 := s               // ERROR E0052: no implicit conversion from i32 to u32
                              //              try: 's as u32' (will panic on negative)

    n := 0
    x := 10 / n               // panic at runtime: division by zero
]
```

### 3.3 Newtypes

**Rule.** `type Name = T` declares **Name** as a type distinct from `T`. `Name` and `T` are not interconvertible without an explicit cast or constructor.

- **Construction.** `Name[value]` constructs a `Name` from a `T`. The compiler checks that `value: T`.
- **Projection.** `name.inner` (or `name as T`) extracts the underlying `T`.
- **No automatic operator forwarding.** Arithmetic on a `Name` requires the programmer to define operators or convert: `Metres[a.inner + b.inner]`. This is deliberate — auto-forwarding would defeat the purpose for unit types (you can add two `Metres`, but you cannot multiply them and get `Metres`).
- **Zero runtime cost.** A newtype has the same layout, size, and ABI as its underlying type.

Range-refined types (`type Altitude = i32 in 0..60_000`) are **deferred** to a future feature, but the syntax is reserved.

**Rationale.** Newtypes give Ada-style unit safety at zero runtime cost. They are the single highest-leverage safety feature for embedded and aerospace code, where mixing meters with feet, or seconds with milliseconds, has caused real disasters (Mars Climate Orbiter, 1999).

**Accepts.**

```drast
type Metres = i32
type Seconds = i32

fn travel_time[d: Metres, v: i32] -> Seconds [
    return Seconds[d.inner / v]
]

fn main [
    d := Metres[1000]
    t := travel_time[d, 10]    // t: Seconds
]
```

**Rejects.**

```drast
type Metres = i32
type Seconds = i32

fn travel_time[d: Metres, v: i32] -> Seconds [ ... ]

fn main [
    s := Seconds[60]
    t := travel_time[s, 10]      // ERROR E0060: expected Metres, found Seconds

    raw: i32 := 1000
    t := travel_time[raw, 10]    // ERROR E0061: expected Metres, found i32
                                 //              try: 'Metres[raw]'
]
```

**Open questions.**
- Whether protocols / traits can be implemented for newtypes independently of the underlying type. Recommended: yes — that's most of the point.
- Operator forwarding policy (`#[derive_add]`-style attributes) for the common case where you really do want `a + b` for `Metres`.
- Range-refined type semantics. To resolve before §3 is closed.

### 3.4 Floats

**Rule.** Drast has exactly two floating-point types: `f32` and `f64`. An untyped float literal (e.g., `3.14`) defaults to `f64`. There is no `float` or `double` keyword.

- **Semantics.** Float operations follow IEEE 754 binary32 / binary64. NaN, ±infinity, signed zero, and denormals behave per the standard.
- **Equality.** `NaN == NaN` is `false`. `NaN != NaN` is `true`. This is required by IEEE 754 and is not a bug. Use `.is_nan[]` for the test.
- **No implicit float↔int conversions.** Every conversion requires `as`. `f64 → f32` may lose precision (no panic; just a truncated result). `f → int` panics if the float is non-finite or out of the target's range, or if it has a fractional part (compiler errors at the cast site request `.floor[].as[i32]` etc.).
- **Safety-profile guard.** In `#profile(safety_critical)`, every `f32`/`f64` value reaching a comparison, an external sink (FFI, print, write), or a type conversion **must be statically or dynamically proven finite**. The compiler emits warnings (errors in strict mode) when a float crosses one of these boundaries without a `.is_finite[]` guard or a function-level annotation asserting finiteness.

**Rationale.** IEEE 754 is what every CPU implements; deviating from it bloats the runtime and breaks legitimate numerical algorithms. Safety profiles add discipline at the boundaries instead — the same approach SPARK takes.

**Accepts.**

```drast
fn main [
    pi := 3.14                    // pi: f64
    small: f32 := 1.0
    big := pi as f32              // explicit narrow

    x := 0.0 / 0.0                // x = NaN; no panic, no UB
    if x.is_nan[] [ print['nan'] ]
]
```

**Rejects.**

```drast
fn main [
    x: f32 := 1                   // ERROR E0070: literal 1 is an integer; use 1.0 or 1 as f32
    y: i32 := 1.0                 // ERROR E0071: no implicit f64 -> i32 conversion
                                  //              try: (1.0).floor[].as[i32]
]
```

### 3.5 Booleans

**Rule.** `bool` has exactly two values: `true` and `false`.

- **No truthiness.** `if`, `while`, `do-while`, and ternary conditions require a value of type `bool`. There is no implicit conversion from integer, pointer, optional, or any other type.
- **No implicit conversion to/from int.** `true as i32` is allowed (yields 1); `1 as bool` is rejected (write `1 != 0` instead).
- **Short-circuit operators.** `&&` and `||` evaluate the right operand only when needed. Both operands must be `bool`.

**Rationale.** C-style truthiness is responsible for two of the top ten bug classes in C/C++ code: `if (x = 5)` (assignment-in-condition) and `if (ptr)` ambiguity between null and intentional zero. Requiring `bool` makes the source say what it means.

**Accepts.**

```drast
fn main [
    ok := true
    if ok [ print['yes'] ]

    n: i32 := 5
    if n != 0 [ print['nonzero'] ]

    u := find_user[42]
    if let user := u [ print[user.name] ]
]
```

**Rejects.**

```drast
fn main [
    n: i32 := 5
    if n [ ... ]                  // ERROR E0080: expected bool, found i32
                                  //              try: 'n != 0'

    u := find_user[42]
    if u [ ... ]                  // ERROR E0081: expected bool, found 'maybe User'
                                  //              try: 'if let user := u'

    x := 1
    if x = 5 [ ... ]              // ERROR E0082: assignment is not an expression
                                  //              (use '==' for comparison)
]
```

### 3.6 Characters and strings

**Rule.**

- **`char`** is a single **Unicode scalar value** in the range `0..=0x10FFFF`, excluding the surrogate range `0xD800..=0xDFFF`. Layout is `u32`. Constructed with `c'x'` (a single Unicode scalar inside `c'...'`). `c'ab'` (more than one scalar) is a compile error.
- **`string`** is an owned, growable, heap-allocated sequence of bytes guaranteed to be **valid UTF-8**. Constructed with `'...'` literals. The literal `'a'` has type `string` even if one byte long — there is no implicit `string ↔ char` conversion.
- **Byte indexing is forbidden in safe code.** `s[i]` is a compile error. Use:
    - `s.bytes[]` for an iterator/slice of `u8`,
    - `s.chars[]` for an iterator of `char`,
    - `s.byte_at[i]` for the byte at position `i` (returns `maybe u8`),
    - explicit string-slice operations (`s.slice[start..end]`) that panic on a non-char-boundary cut.
- **Borrowing.** `~string` is a borrowed string slice (analogous to Rust `&str`). A string literal `'hello'` evaluated in an expression position has type `~string` (it points into the program's static data).
- **Mutation.** Mutating a string (push, insert, replace) requires `mut` ownership: `mut s := 'hi'; s.push_str['!']`.

**Rationale.** UTF-8 is the universal modern encoding; forbidding byte-indexing prevents the entire family of "I cut a code point in half" bugs (a frequent source of crashes in C/C++ string handling). The `'x'` literal being always a string was a design decision the author made earlier; this section codifies it.

**Accepts.**

```drast
fn main [
    s := 'héllo'                  // s: ~string (static); 6 bytes, 5 chars
    n := s.byte_count[]           // 6
    c := s.char_count[]           // 5

    c0 := c'h'                    // c0: char
    c1 := c'é'                    // c1: char (single Unicode scalar, even though 2 bytes UTF-8)

    for ch in s.chars[] [ print[ch] ]

    mut owned := 'hi'             // owned: string
    owned.push_str['!']           // OK; 'hi!'
]
```

**Rejects.**

```drast
fn main [
    s := 'hello'
    b := s[0]                     // ERROR E0090: 'string' is not indexable by integer
                                  //              try: s.bytes[][0] or s.chars[].nth[0]

    c := c'ab'                    // ERROR E0091: 'c'...'' must contain exactly one Unicode scalar

    s := 'hi'
    s.push_str['!']               // ERROR E0092: cannot mutate immutable binding 's'
                                  //              declare as 'mut s := ...'
]
```

**Open questions.**
- Whether string interpolation (e.g., `'count = $n'`) is built into the literal or done via a formatter. Recommended: built-in interpolation with explicit type-driven formatting via a stdlib protocol.
- Whether raw strings and multi-line strings get a distinct delimiter (e.g., triple-`'`).
- Grapheme cluster iteration (`.graphemes[]`) — required for user-facing text but heavy in stdlib. Defer to stdlib design.

### 3.7 Arrays, slices, and collection types
*To be decided in next round.*

## 4. Expressions and evaluation
*To be decided.*

## 5. Statements and control flow
*To be decided.*

## 6. Declarations
*To be decided.*

## 7. Modules and visibility
*To be decided.*

## 8. C++ FFI and `unsafe`
*To be decided.*

## 9. Safety profiles
*To be decided.*

## 10. What the compiler MUST reject
*To be decided.*
