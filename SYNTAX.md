# Drast Syntax Reference

This document describes the language implemented by the current self-hosted Drast transpiler. It documents the syntax that is accepted by the lexer, parser, and code generator, including implementation quirks.

<!-- SEMANTICS.md is the language contract. Entries below that describe legacy C++ bootstrap behavior are retained as historical syntax notes only; when this file and SEMANTICS.md disagree, SEMANTICS.md wins and the type checker must reject the legacy form. -->

## Source Model

Drast is indentation-sensitive. A larger indentation level emits an `Indent` token and a smaller level emits one or more `Dedent` tokens. Tabs count as four spaces. Blank lines and comments are skipped by the lexer.

Line comments use `//`. Block comments use `/* ... */` and may nest.

Statements normally end at a newline, dedent, or end of file. Semicolons are not C-style statement terminators; they are used by Drast for parameter/type separators, field payload separators, labeled arguments, and optional declaration markers.

## Keywords

Implemented keywords:

- `use`: import a Drast module, opt into the Drast prelude, or emit a C++ include.
- `const`: marks a top-level global as C++ `const`.
- `return`: emits a C++ `return`.
- `if`, `elif`, `else`: conditional control flow.
- `while`: loop while a condition is true.
- `for`, `in`, `to`, `until`, `step`: range and foreach loops.
- `break`, `continue`: loop control.
- `struct`: declares a C++ `struct`.
- `enum`: declares a C++ `enum class` or tagged data enum.
- `impl`: declares methods for a struct.
- `protocol`: declares a C++ abstract base class.
- `match`, `default`: pattern-style dispatch over values and data enums.
- `with`: parsed after a function declaration and skipped; generic constraints are not enforced.
- `as`: used in `impl Type as Protocol`.
<!-- SEMANTICS.md §1.4 contradiction: Drast has Result/try propagation, not exception blocks. Legacy `try`/`catch` syntax is rejected by the semantic checker with E0030. -->
- `try`, `catch`: emits C++ `try`/`catch`.
- `private`: marks struct fields private and is accepted on methods.
- `preview`: accepted as a field visibility marker, emitted like public.
- `fileprivate`: accepted before top-level `struct` or `enum`; currently stored but not enforced in emitted C++.
- `discard`: accepted after a function return type; stored but not used for warning enforcement.
- `operator`: declares an overloaded operator in an `impl`.
- `maybe`: wraps a function return type in `std::optional<T>`.
- `nothing`: no-op statement placeholder for intentionally empty blocks.
<!-- SEMANTICS.md §1.5 contradiction: safe Drast has `None`, not `nil`; `nil` is rejected by the semantic checker with E0042. -->
- `true`, `false`, `nil`, `self`: boolean, optional-null, and method receiver expressions.
- `variadic`: type constructor for `std::initializer_list<T>`.
- `tuple`: type and expression marker for `std::tuple` / `std::make_tuple`.
- `iseq`, `isne`, `islt`, `isgt`, `islteq`, `isgteq`, `and`, `or`, `not`: word operators.
- `shl`, `shr`, `bor`, `band`, `bxor`: lexed as keywords but not currently parsed as expression operators.

## Imports: `use`

`use` is handled before other top-level declarations in the containing file. Imports are loaded immediately, so declarations from imported modules are available to later files and to final C++ emission.

Forms:

```drast
use std
use drast
use no_runtime
use helper
use lib/math
use '../shared/tools'
use '/absolute/path/to/module.drast'
use 'native.hpp'
use file native.hpp
use iostream as std
use vector
use Arduino.h
```

Rules:

- `use drast` loads `drast_flavour.drast`, the opt-in Drast prelude for helper idioms such as `println`, file IO, diagnostics, and standard convenience functions.
- `use std` is reserved for native C++ standard-library interop and does not load the Drast prelude.
- `use no_runtime` suppresses generated CLI argument setup for legacy sources that request it.
- A non-header module path resolves relative to the importing file's directory unless it starts with `/`.
- Missing `.drast` suffixes are added during resolution.
- `.` and `..` path components are normalized.
- Duplicate module loads are skipped.
- Circular imports are diagnosed with `circular import`.
- Header paths ending in `.h`, `.hpp`, `.hh`, or `.hxx` emit C++ `#include` lines instead of loading Drast.
- Standard C++ headers such as `use vector` emit angle includes, and known stdlib type names map to their `std::` spellings.
- `use iostream as std` emits `#include <iostream>` and namespace member access like `std.cout` as `std::cout`.
- Quoted header uses and `use file ...` emit `#include "..."`.
- Unquoted header uses emit `#include <...>`.
- Duplicate include lines are suppressed.
- Angle include syntax such as `use <vector>` is not lexed.

Example:

```drast
use drast
use math
use file native_add.hpp

main, int
	answer = increment 41
	println answer
	return 0
```

Loads the Drast prelude, emits a quoted C++ include for `native_add.hpp`, and loads `math.drast` from the same directory.

## Type System

Primitive names map as follows:

<!-- SEMANTICS.md §3.1 and §3.4 contradiction: `int`, `uint`, `float`, and `double` are legacy bootstrap names. The semantic checker accepts fixed-width integer types and `f32`/`f64`; legacy names are rejected with E0053. -->
- `int` / `Int` -> `int`
- `float` / `Float` -> `float`
- `double` / `Double` -> `double`
- `bool` / `Bool` -> `bool`
- `char` / `Char` -> `char`
- `string` / `String` -> `std::string`
- `usize` -> `std::size_t`

Other type names are emitted unchanged except `.` is converted to C++ `::`.

Type constructors:

```drast
name string
items {int}
borrowed ~Box
pointer string`
heap @[Box]
values variadic[int]
pair tuple Int String
counts map`[string int]
```

Mapping:

- `{T}` -> `std::vector<T>`
- `~T` -> `T&`
- `T`` -> `T*`; multiple backticks add more pointer depth.
- `@[T]` -> `std::shared_ptr<T>`
- `variadic[T]` -> `std::initializer_list<T>`
- `tuple A B` -> `std::tuple<A, B>`
- `map`[K V]` -> `std::unordered_map<K, V>`
- `Type`[A B]` -> `Type<A, B>`

<!-- SEMANTICS.md §1.5 contradiction: `maybe T` is a semantic sum type with `Some[T]` / `None`; the current C++ backend may still lower it through `std::optional<T>` during bootstrap. -->
Function return `maybe T` maps to `std::optional<T>`.

## Declarations

### Globals

Top-level identifier lines with `=` or `const` are globals.

```drast
answer = 42
limit const = 100
name string = 'Drast'
```

Mapping:

```cpp
int answer = 42;
const int limit = 100;
std::string name = "Drast";
```

The type is inferred from the initializer when omitted, falling back to `auto` if unknown.

<!-- SEMANTICS.md §1.3 contradiction: module-level declarations are API boundaries and require explicit type annotations. Untyped globals are rejected by the semantic checker with E0022. -->

### Functions

Function syntax:

```drast
name param;Type paramWithDefault;Type;defaultValue, ReturnType discard
	body
```

No parentheses are used. Parameters are separated by spaces. A comma introduces the return type. No comma means `void`.

Examples:

```drast
increment value;int, int
	return value + 1

createPoint x;int;0 y;int;0, tuple Int Int
	return tuple x y

parse text;string, maybe int
	return text.asInt
```

Generic functions use a backtick parameter list:

```drast
identity`[T] value;T, T
	return value
with T as Copyable
```

The `with` line is consumed and skipped; constraints are not enforced.

### Structs

```drast
struct Point
	x float
	y float
	private cachedLength float
```

Public and `preview` fields are emitted public. `private` fields are emitted under a C++ `private:` section. Constructors are generated from non-private fields unless the struct has an `init` method.

Generic structs:

```drast
struct Box`[T]
	value T
```

Emits a C++ template.

### Impl Blocks

```drast
impl Point
	init x;float y;float
		self.x = x
		self.y = y

	move dx;float dy;float
		self.x += dx
		self.y += dy

	deinit
		println 'destroyed'
```

Mapping:

- `init` emits a C++ constructor.
- `deinit` emits a C++ destructor.
- Other methods emit `Return Host::method(...)`.
- `self` maps to `this`.

Protocol conformance:

```drast
impl Dog as Animal
	makeSound, string
		return 'sound.mp3'
```

The struct emits `: public Animal`. The current implementation records conformance with a synthetic marker method.

Operators:

```drast
impl Point
	operator[==] left;Point right;Point, bool
		return left.x == right.x and left.y == right.y
```

With no parameters, `operator[==]` defaults to two `const Host&` parameters named `_1` and `_2`.

### Protocols

```drast
protocol Animal
	makeSound, string
	getScientificName, string
```

Emits an abstract C++ struct with a virtual destructor and pure virtual methods.

### Enums

Simple enum:

```drast
enum Direction
	North; South; East; West;
```

Emits:

```cpp
enum class Direction { North, South, East, West };
```

Multi-word variants are joined with underscores:

```drast
enum Dog.Breed
	Golden Retriever
```

Emits nested enum value `Golden_Retriever` inside `Dog::Breed`.

Data enum:

```drast
enum Expr
	Number value;int
	Add left;Expr right;Expr
```

Emits payload structs, a `Tag`, a `std::variant`, and static constructors such as `Expr::Number(int value)`.

## Control Flow

### If / Elif / Else

```drast
if value islt 10
	println 'small'
elif value islt 100
	println 'medium'
else
	println 'large'
```

Emits C++ `if`, `else if`, and `else` blocks. Local type maps are restored after each branch.

### While

```drast
while index islt limit
	index += 1
```

Emits `while (...)`.

### For

Foreach:

```drast
for word in words
	println word
```

Emits `for (auto& word : words)`.

Inclusive range:

```drast
for i in 1 to 10 step 2
	println i
```

Emits `i <= end`.

Exclusive range:

```drast
for i in 0 until count
	println i
```

Emits `i < end`.

### Match

Value match:

```drast
match name
	'Ash'
		println 'known'
	default
		println 'unknown'
```

Emits a local `_match` reference and chained `if` / `else if` comparisons.

Data enum match:

```drast
match expr
	Expr.Number value
		return value
	Expr.Add left right
		return [eval left] + [eval right]
```

For known data enum variants, matching checks `_match.tag` and binds payload fields from `std::get<Enum_Variant>(_match.data)`.

### Try / Catch

<!-- SEMANTICS.md §1.4 contradiction: exception blocks are not Drast. Use `Result[T, E]` and prefix `try expr`; legacy `try`/`catch` is rejected with E0030. -->

```drast
try
	value = maybe.value
catch ;error
	println error
```

Emits:

```cpp
try { ... } catch (const std::exception& error) { ... }
```

Without `;name`, the binding defaults to `_1`.

## Variables and Assignment

Typed declaration with initializer:

```drast
name string = 'Drast'
```

Untyped declaration with inference:

```drast
name = 'Drast'
```

Uninitialized declaration:

```drast
result int;
```

Compound assignment:

```drast
count += 1
name += '!'
```

For `std::vector<T>`, `items += value` emits `items.push_back(value)`.

Optional unwrapping quirk: assigning an optional expression to a non-optional target emits `.value_or(default)`.

<!-- SEMANTICS.md §1.5 contradiction: implicit optional unwrapping is forbidden. Use `match`, `if let`, prefix `try`, `->force[]`, `->force_with[msg]`, or `.value_or[default]`; treating `maybe T` as `T` is rejected with E0040/E0041. -->

## Expressions

Precedence, from lowest to highest:

1. `or`
2. `and`
3. equality: `==`, `iseq`, `isne`
4. comparisons: `islt`, `isgt`, `islteq`, `isgteq`, and identifier words `islt`, `isgt`, `islte`, `isgte`
5. addition/subtraction: `+`, `-`
6. multiplication/division/remainder: `*`, `/`, `%`
7. unary: `not`, `-`, `~`, backtick dereference
8. postfix: field access, indexing, constructor/call brackets, generic arguments

Operators map to C++:

- `and` -> `&&`
- `or` -> `||`
- `not` -> `!`
- `iseq` / `==` -> `==`
- `isne` -> `!=`
- `islt` -> `<`
- `isgt` -> `>`
- `islteq` / `islte` -> `<=`
- `isgteq` / `isgte` -> `>=`

### Calls

Drast uses implicit calls:

```drast
println 'hello'
add 1 2
```

Emits a generated support call for `println("hello")` and a direct `add(1, 2)` call.

Bracket constructor/call syntax:

```drast
point = Point[10 20]
number = Float[getInput]
```

If the callee is type-like, brackets emit constructor/cast syntax. For `Float[...]`, the current code emits `std::stof(...)`.

Batch calls:

```drast
println[
	'one',
	'two'
]
```

Emits separate calls for each comma-separated batch.

Named/labeled call arguments use `;label value`; labels are consumed and ignored by the emitter.

Positional argument references use `;1`, `;2`, etc. They emit `_1`, `_2`, etc.

### Field Access and Built-ins

`object.field` emits `object.field` or `object->field` for heap/shared-pointer types. `self.field` emits `this->field`.

Special field/method mappings:

- `text.length` / `array.length` -> `.size()`
- `text.lineCount` -> generated line-count support
- `text.splitWhitespace` -> generated whitespace-splitting support
- `text.trim` -> generated string-trim support
- `text.asInt` -> generated integer-parse support
<!-- SEMANTICS.md §1.5 contradiction: optional `.value` is not a safe unwrap surface. The semantic checker rejects field access on `maybe T` with E0040. -->
- `optional.value` -> `.value()`
- `text.lowercase` -> generated lowercase support
- `map.keys` -> generated key-collection support
- `map.values` -> generated value-collection support
- `.contains`, `.startsWith`, `.endsWith`, `.find`, `.replace`, `.split`, `.get`, `.set`, `.clear`, `.removeAt`, `.remove`, `.substring`, `.valueOr` map to generated support or direct STL calls.

Standard runtime calls recognized by name include `print`, `println`, `printf`, `getInput`, `arg`, `readFile`, `writeFile`, `fileExists`, `args`, `toString`, `parseInt`, `parseFloat`, `charCode`, character classification helpers, and diagnostic helpers.

### Literals

```drast
42
3.14
'a'
'hello'
s'a'
true
false
nil
{1 2 3}
{int}
@[Box 42]
```

Rules:

<!-- SEMANTICS.md §3.1, §3.4, and §3.6 contradictions: integer literals default to `i32`, float literals default to `f64`, `'...'` is always `string`, `c'...'` is `char`, and `nil` is not safe Drast. The bullets below describe legacy C++ bootstrap emission only. -->
- Numbers with a decimal point are `float`; others are `int`.
- Single quotes are used for both strings and chars.
- A quoted literal with one character is a C++ `char`; longer text is `std::string`.
- Prefix `s'...'` forces a string literal.
- `nil` emits `std::nullopt`.
- `{expr ...}` emits a C++ initializer list and is typed as `std::vector<Element>` when element type is known.
- `{Type}` with a single type-like item emits an empty `std::vector<Type>`.
- `@[Type args...]` emits `std::make_shared<Type>(args...)`.

Escapes supported in quoted literals: `\n`, `\t`, `\\`, `\'`, `\"`, `\0`, and `\r`.

## `nothing` - Empty Block Placeholder

Use `nothing` as a statement inside any block that is intentionally empty. It is a no-op and compiles to a C++ null statement.

Examples:

```drast
while true
	nothing

if someCondition
	doSomething
else
	nothing

placeholder
	nothing
```

`nothing` is only valid at statement position. Using it as an expression is a compile error.

## C++ Emission Order

The code generator emits:

1. Runtime and requested `#include` lines.
2. Forward declarations for structs and data enums.
3. Protocol declarations.
4. Non-nested enums.
5. Struct declarations, including nested simple enums and method declarations.
6. Globals.
7. Method definitions.
8. Forward declarations for non-`main` top-level functions.
9. Non-`main` function definitions.
10. `main` last.

If a program uses `args` or `arg`, emitted C++ `main` receives `int argc, char **argv` and initializes the generated argument support before calling user code.

## Known Limitations and Quirks

- Parsing and statement body emission are still intertwined: `Parser` builds AST declarations but also stores already-emitted C++ strings for function bodies.
- `preview`, `fileprivate`, `discard`, and `with` constraints are accepted but mostly informational.
- Bitwise word tokens (`shl`, `shr`, `bor`, `band`, `bxor`) are lexed but not parsed as binary operators.
- Angle include syntax is not supported.
- Function overloading is emitted as plain C++ overloads, but Drast does not perform overload resolution itself.
- Generic constraints after `with` are skipped, and generic inference is delegated to emitted C++ templates where possible.
- Enum shorthand `.Variant` resolves through a global variant-name map; duplicate variant names across enums can collide.
- Multi-word enum variants are joined with underscores.
- Module identity is normalized string paths, not filesystem-realpath identity; symlinked cycles are not specially collapsed.
- Header includes are deduplicated by exact emitted include line.
- Some example files are design sketches and intentionally exercise syntax beyond the currently reliable test suite.
