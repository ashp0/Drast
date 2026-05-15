# Drast C++ Code Generation Audit

Audit date: 2026-05-15

This audit used the installed stable binary:

```sh
transpiler help
bash tests/audit_cpp_codegen/run_audit.sh
```

The audit corpus is in `tests/audit_cpp_codegen/cases/`. Captured C++ is in `tests/audit_cpp_codegen/generated/`. Build outputs and compiler diagnostics are in `tests/audit_cpp_codegen/build/`.

## Build Results

| Case | Result | Notes |
| --- | --- | --- |
| `data_enum_match` | C++ build failed | Data enum match lowers to invalid equality checks and undeclared payload names. |
| `generics` | C++ build failed | Generic return type inference leaks template placeholder `T` into `main`. |
| `loops_and_maps` | Built with warning | `counts.set word current + 1` emits wrong C++ and wrong runtime behavior. |
| `functions_and_collections` | Built | Mostly good parameter lowering, but still exposes loop constness and copy policy opportunities. |
| `struct_impl` | Built | Constructor and method output is correct but not professional C++ style. |
| `memory_operations` | Built | Shows raw pointer exposure and unconditional shared ownership. |
| `optionals_and_errors` | Built | Uses exceptions for ordinary optional absence. |
| `protocols_and_operators` | Built | Missing `const` and `override` on protocol methods. |
| `interop` | Built | Simple free-function header interop is acceptable. |
| `value_match` | Built | String match output is acceptable. |

## AUDIT-001: Data enum match emits invalid C++

Triggered by `tests/audit_cpp_codegen/cases/data_enum_match/main.drast`:

```drast
enum Message
	Text body;string
	Count amount;int

describe message;~Message, string
	match message
		Message.Text body
			return body
		Message.Count amount
			return s'count'
```

Bad C++ output:

```cpp
std::string describe(Message& message) {
    {
        const auto& _match = message;
        if (_match == Message::Text(body)) {
            return body;
        }
        else if (_match == Message::Count(amount)) {
            return "count";
        }
    }
}
```

This does not compile. `body` and `amount` are not in scope, `Message` has no `operator==`, and the payload is never extracted from the `std::variant`.

Ideal C++ output:

```cpp
std::string describe(const Message& message) {
    switch (message.tag) {
    case Message::Tag::Text: {
        const auto& payload = std::get<Message_Text>(message.data);
        const std::string& body = payload.body;
        return body;
    }
    case Message::Tag::Count: {
        const auto& payload = std::get<Message_Count>(message.data);
        const int amount = payload.amount;
        return "count";
    }
    }
    __builtin_unreachable();
}
```

Transpiler change:

Build a separate lowering path for data enum matches. Once the scrutinee type is a known data enum, emit `switch (_match.tag)` or tag-based `if` checks, bind payload fields from `std::get<VariantPayload>(_match.data)`, and mark the match exhaustive when all tags are covered.

## AUDIT-002: Data enum constructor inference chooses the variant name as the variable type

Triggered by `tests/audit_cpp_codegen/cases/data_enum_match/main.drast`:

```drast
message = Message.Text[s'hello']
```

Bad C++ output:

```cpp
Text message = Message::Text("hello");
```

This does not compile. The variable type must be the enum type, not the variant case name.

Ideal C++ output:

```cpp
Message message = Message::Text("hello");
```

Transpiler change:

When inferring from a data enum static constructor expression, record the owner enum type as the expression type. The expression `Enum.Case[...]` should type as `Enum`, never `Case`.

## AUDIT-003: Generic calls leak template placeholders into non-template code

Triggered by `tests/audit_cpp_codegen/cases/generics/main.drast`:

```drast
text = identity`[string] box.value
first = firstValue`[int] numbers
```

Bad C++ output:

```cpp
T text = identity(box.value);
T first = firstValue(numbers);
```

This does not compile because `T` is not declared in `main`.

Ideal C++ output:

```cpp
std::string text = identity<std::string>(box.value);
int first = firstValue<int>(numbers);
```

Acceptable output if the type resolver cannot yet materialize concrete types:

```cpp
auto text = identity<std::string>(box.value);
auto first = firstValue<int>(numbers);
```

Transpiler change:

Substitute explicit generic arguments into the generic function signature before assigning an expression type. If substitution fails, emit `auto` for local inference rather than leaking template parameter names outside their template scope.

## AUDIT-004: Generic struct locals rely on bare CTAD-style spelling

Triggered by `tests/audit_cpp_codegen/cases/generics/main.drast`:

```drast
box = Box`[string][s'payload']
```

Bad C++ output:

```cpp
Box box = Box<std::string>("payload");
```

This is fragile and visually odd. Even when class template argument deduction can recover, professional generated C++ should not hide the instantiated type.

Ideal C++ output:

```cpp
Box<std::string> box{"payload"};
```

Transpiler change:

For generic constructor expressions, carry the instantiated type through local type inference. Emit the full type on the declaration or emit `auto` consistently:

```cpp
auto box = Box<std::string>{"payload"};
```

## AUDIT-005: Method call argument parsing changes program semantics

Triggered by `tests/audit_cpp_codegen/cases/loops_and_maps/main.drast`:

```drast
counts.set word current + 1
```

Bad C++ output:

```cpp
(counts[word] = current) + 1;
```

This compiles with `-Wunused-value` and produces the wrong result. The intended increment is discarded.

Ideal C++ output:

```cpp
counts[word] = current + 1;
```

Better output for this specific map update:

```cpp
++counts[word];
```

Transpiler change:

Argument parsing for postfix/runtime-helper calls must consume a full expression for each argument, including additive and multiplicative tails. The map `.set` emitter should also parenthesize the full RHS expression before emitting assignment.

## AUDIT-006: Map helpers emit repeated lookups and allocate key vectors for iteration

Triggered by `tests/audit_cpp_codegen/cases/loops_and_maps/main.drast`:

```drast
for word in words
	if counts.contains word
		current = counts.get word 0
		counts.set word current + 1
	else
		counts.set word 1
keys = counts.keys
for key in keys
	total += counts.get key 0
```

Bad C++ output:

```cpp
if (drast::contains(counts, word)) {
    auto current = drast::map_get(counts, word, 0);
    (counts[word] = current) + 1;
} else {
    (counts[word] = 1);
}
std::vector<std::string> keys = drast::map_keys(counts);
for (auto& key : keys) {
    total += drast::map_get(counts, key, 0);
}
```

Even after AUDIT-005 is fixed, this performs extra hash lookups and allocates a new vector of keys.

Ideal C++ output:

```cpp
for (const auto& word : words) {
    auto [it, inserted] = counts.try_emplace(word, 0);
    ++it->second;
}

int total = 0;
for (const auto& [key, value] : counts) {
    total += value;
}
```

Transpiler change:

Teach the map helper lowering about common read-modify-write and key-iteration patterns. `.keys` should not allocate when the only consumer is a `for` loop; lower directly to map iteration or a lightweight view.

## AUDIT-007: Read-only loops still emit mutable-looking references

Triggered by `tests/audit_cpp_codegen/cases/functions_and_collections/main.drast` and `tests/audit_cpp_codegen/cases/struct_impl/main.drast`:

```drast
for value in values
	total += value

for entry in self.entries
	sum += entry
```

Bad C++ output:

```cpp
for (auto& value : values) {
    total += value;
}

for (auto& entry : this->entries) {
    sum += entry;
}
```

In a `const std::vector<int>&` loop this deduces to `const int&`, but it still reads as mutable. In non-const methods it actually is mutable even when the loop body does not mutate the element.

Ideal C++ output:

```cpp
for (const auto& value : values) {
    total += value;
}

for (const auto& entry : this->entries) {
    sum += entry;
}
```

Transpiler change:

Add loop body mutation analysis. If the loop variable is not assigned to, passed as `~T`, or used in another mutating operation, emit `const auto&`.

## AUDIT-008: `impl init` constructors default-construct fields then assign

Triggered by `tests/audit_cpp_codegen/cases/struct_impl/main.drast`:

```drast
impl Ledger
	init title;string entries;{int}
		self.title = title
		self.entries = entries
		self.cached = 0
```

Bad C++ output:

```cpp
Ledger::Ledger(const std::string& title, const std::vector<int>& entries) {
    this->title = title;
    this->entries = entries;
    this->cached = 0;
}
```

This default-constructs `title` and `entries`, then copy-assigns them. For vectors and strings, that is unnecessary work.

Ideal C++ output:

```cpp
Ledger::Ledger(std::string title, std::vector<int> entries)
    : title(std::move(title)),
      entries(std::move(entries)),
      cached(0) {}
```

Transpiler change:

Recognize constructor bodies that assign parameters directly to fields and lower them into initializer lists. For owned fields of movable types, take parameters by value and move into fields. Preserve `const&` only for borrowed fields or when the parameter is not stored.

## AUDIT-009: Read-only methods are not marked `const`

Triggered by `tests/audit_cpp_codegen/cases/struct_impl/main.drast`:

```drast
impl Ledger
	total, int
		sum = 0
		for entry in self.entries
			sum += entry
		return sum
```

Bad C++ output:

```cpp
int Ledger::total() {
    int sum = 0;
    for (auto& entry : this->entries) {
        sum += entry;
    }
    return sum;
}
```

Professional C++ readers expect observers to be `const`.

Ideal C++ output:

```cpp
int Ledger::total() const {
    int sum = 0;
    for (const auto& entry : entries) {
        sum += entry;
    }
    return sum;
}
```

Transpiler change:

Analyze method bodies for writes through `self`, non-const reference calls on fields, and calls to non-const methods. If none are present, mark the declaration and definition `const`.

## AUDIT-010: Borrowed read-only parameters are emitted as mutable references

Triggered by `tests/audit_cpp_codegen/cases/memory_operations/main.drast`:

```drast
readNode node;~Node, int
	return node.value
```

Bad C++ output:

```cpp
int readNode(Node& node) {
    return node.value;
}
```

The function cannot accept a `const Node`, and callers cannot tell that the function is read-only.

Ideal C++ output:

```cpp
int readNode(const Node& node) {
    return node.value;
}
```

Transpiler change:

Do not map every `~T` parameter directly to `T&`. Use body analysis: if the parameter is never mutated, passed to a mutating borrow, or stored as mutable, emit `const T&`.

## AUDIT-011: Protocol implementations omit `const` and `override`

Triggered by `tests/audit_cpp_codegen/cases/protocols_and_operators/main.drast`:

```drast
protocol Shape
	area, int

impl Rect as Shape
	area, int
		return self.width * self.height
```

Bad C++ output:

```cpp
struct Shape {
    virtual ~Shape() = default;
    virtual int area() = 0;
};

struct Rect : public Shape {
    int area();
};

int Rect::area() {
    return this->width * this->height;
}
```

The method is an observer but not `const`, and the implementation lacks `override`.

Ideal C++ output:

```cpp
struct Shape {
    virtual ~Shape() = default;
    virtual int area() const = 0;
};

struct Rect : public Shape {
    int area() const override;
};

int Rect::area() const {
    return width * height;
}
```

Transpiler change:

Run the same const analysis for protocol methods and implementations. When a method implements a protocol requirement, emit `override` on the derived declaration. Reject signature drift instead of silently emitting a new overload.

## AUDIT-012: Optional absence is modeled with broad exception handling

Triggered by `tests/audit_cpp_codegen/cases/optionals_and_errors/main.drast`:

```drast
forceEven value;int, int
	candidate = findEven value
	try
		return candidate.value
	catch ;error
		return 0
```

Bad C++ output:

```cpp
int forceEven(int value) {
    std::optional<int> candidate = findEven(value);
    try {
        return candidate.value();
    } catch (const std::exception& error) {
        return 0;
    }
}
```

This uses exception control flow for an ordinary missing optional and catches every `std::exception`.

Ideal C++ output:

```cpp
int forceEven(int value) {
    const std::optional<int> candidate = findEven(value);
    return candidate.value_or(0);
}
```

If the source explicitly needs a catch block, narrow the catch:

```cpp
try {
    return candidate.value();
} catch (const std::bad_optional_access&) {
    return 0;
}
```

Transpiler change:

Lower common `try { optional.value } catch { fallback }` shapes to `value_or` or an `if (candidate)` branch. If preserving `try/catch`, catch `std::bad_optional_access` for optional unwraps instead of `std::exception`.

## AUDIT-013: Raw pointer output appears for non-escaping local borrows

Triggered by `tests/audit_cpp_codegen/cases/memory_operations/main.drast`:

```drast
pointer = ~label
copied = `pointer
```

Bad C++ output:

```cpp
std::string* pointer = &label;
std::string copied = *pointer;
```

The user had to think in address-of and dereference operations, and the generated C++ exposes a raw pointer even though the borrow never escapes.

Ideal C++ output:

```cpp
const std::string& pointer = label;
std::string copied = pointer;
```

Better output if the alias is only used once:

```cpp
std::string copied = label;
```

Transpiler change:

Treat `~expr` as a borrow operation in the semantic model, not as immediate raw pointer syntax. If the borrow is non-null, non-reassigned, and non-escaping, emit a reference or eliminate the alias. Emit raw pointers only for nullable borrows, rebindable borrows, C interop, or explicit pointer types.

## AUDIT-014: Heap allocation always uses shared ownership

Triggered by `tests/audit_cpp_codegen/cases/memory_operations/main.drast`:

```drast
heap = @[Node 9 copied]
alias = heap
return readNode stack + alias.value
```

Bad C++ output:

```cpp
std::shared_ptr<Node> heap = std::make_shared<Node>(9, copied);
std::shared_ptr<Node> alias = heap;
return readNode(stack) + alias->value;
```

This pays for atomic shared ownership even when the program only needs one owner plus a temporary alias.

Ideal C++ output:

```cpp
auto heap = std::make_unique<Node>(9, copied);
const Node& alias = *heap;
return readNode(stack) + alias.value;
```

If the value truly escapes to multiple owners:

```cpp
auto heap = std::make_shared<Node>(9, copied);
std::shared_ptr<const Node> alias = heap;
```

Transpiler change:

Infer ownership for `@[T ...]`:

1. Use `std::unique_ptr<T>` when there is a single owner.
2. Use references for non-owning aliases of a live owner.
3. Use `std::shared_ptr<T>` only when ownership is copied into independently live variables, containers, returned values, or closures.
4. Use `std::shared_ptr<const T>` when all observed uses are read-only.

## AUDIT-015: Move semantics are not inferred at last use

Triggered by `tests/audit_cpp_codegen/cases/memory_operations/main.drast`:

```drast
stack = Node[7 copied]
heap = @[Node 9 copied]
```

Bad C++ output:

```cpp
Node stack = Node(7, copied);
std::shared_ptr<Node> heap = std::make_shared<Node>(9, copied);
```

`copied` is not used after the heap construction, so the last construction can move the string payload.

Ideal C++ output:

```cpp
Node stack{7, copied};
auto heap = std::make_unique<Node>(9, std::move(copied));
```

Transpiler change:

Add last-use analysis for local variables with movable non-trivial types. Emit `std::move(local)` only when the variable is not used afterward, is not `const`, and is not a borrow/reference alias.

## AUDIT-016: Match lowering can fall off the end of non-void functions

Triggered by `tests/audit_cpp_codegen/cases/data_enum_match/main.drast`:

```drast
describe message;~Message, string
	match message
		Message.Text body
			return body
		Message.Count amount
			return s'count'
```

Bad C++ output:

```cpp
std::string describe(Message& message) {
    {
        const auto& _match = message;
        if (...) {
            return body;
        }
        else if (...) {
            return "count";
        }
    }
}
```

Even after the invalid enum check is fixed, an `if` chain does not prove to C++ that all paths return.

Ideal C++ output:

```cpp
std::string describe(const Message& message) {
    switch (message.tag) {
    case Message::Tag::Text:
        return std::get<Message_Text>(message.data).body;
    case Message::Tag::Count:
        return "count";
    }
    __builtin_unreachable();
}
```

Transpiler change:

Track match exhaustiveness. For data enums, know the full variant set. For simple enums, know the full case set. If exhaustive, emit a `switch` plus unreachable marker. If not exhaustive and the surrounding function is non-void, require `default` or emit a compile-time diagnostic.

## AUDIT-017: Closure support is absent, so closure C++ cannot be audited

`SYNTAX.md` documents functions, generics, protocols, match, optionals, and interop, but no closure or lambda syntax. That means Drast cannot currently express a local function value with capture.

Potential Drast source Drast should eventually support:

```drast
main, int
	factor = 2
	twice = value;int => value * factor
	return twice 21
```

Current C++ output:

```cpp
// No C++ is emitted because there is no accepted closure syntax.
```

Ideal C++ output:

```cpp
int main(int argc, char **argv) {
    drast::setArgs(argc, argv);
    const int factor = 2;
    auto twice = [factor](int value) { return value * factor; };
    return twice(21);
}
```

Transpiler change:

Add a closure AST node with explicit parameter and return type inference. Capture immutable locals by value, mutable borrows by reference only when lifetime-safe, and captured move-only owners by move capture. Lower generic closures to templated call operators only when the language has a clear syntax for that power.

## Memory Management Rules

Drast should let users write high-level ownership intent and should make the C++ backend carry the burden. Concrete rules:

1. Plain locals and fields should default to value semantics. Emit stack objects unless the value escapes, is recursive, or is explicitly heap allocated.
2. `~T` should mean a borrow, not necessarily `T*` or `T&`. Emit `const T&` for read-only borrows, `T&` for mutable borrows, and reject escaping borrows unless the owner clearly outlives the escape.
3. Raw pointers should be an interop escape hatch. Emit `T*` only for nullable/rebindable borrows, C/C++ API boundaries, or explicit pointer annotations.
4. `@[T ...]` should not always mean `std::shared_ptr<T>`. Default to `std::unique_ptr<T>` for single-owner heap values. Promote to `std::shared_ptr<T>` only when ownership is copied into independent lifetimes.
5. A copied heap handle used only as an alias should become `T&` or `const T&`, not another owner.
6. Constructor and struct field initialization should use initializer lists. Owned fields should receive by value and move, or use forwarding constructors where the emitted C++ remains readable.
7. Emit `std::move` at proven last use for movable owned locals. Never move from borrowed references, `const` values, globals, or variables used later.
8. Optionals should use `std::optional<T>` without exception control flow for ordinary absence. Use `value_or`, `if (opt)`, or pattern matching. Reserve exceptions for actual C++ exception interop.
9. Data enum payloads should move into variants on construction and bind by `const&` in read-only matches. Bind by mutable reference only when the match arm mutates the payload.
10. Containers should avoid allocating helper vectors for `.keys`, `.values`, and similar views when consumed immediately by loops. Prefer direct iteration or view objects.

## Future-Proofing Priorities

1. Build a typed middle IR before C++ emission. Today the backend still shows parser/codegen coupling through bugs like `T text` and data enum match lowering. A typed IR should know expression type, value category, ownership, mutability, and exhaustiveness before any C++ string is produced.
2. Add C++ quality gates to the test suite. Every audit case should run `transpiler transpile`, `transpiler build`, and then compile with `-Wall -Wextra -Werror` for generated C++. Snapshot the generated C++ for intentional style review.
3. Make constness a first-class analysis. Functions, methods, loops, protocol requirements, match payload bindings, and borrowed parameters all need the same const/mutation facts.
4. Make ownership a first-class analysis. The backend should choose values, references, `unique_ptr`, `shared_ptr`, and moves from semantic facts instead of mirroring surface syntax.
5. Prefer pattern-specialized lowering for standard helpers. Map counting, key iteration, string splitting, optional fallback, and data enum matches should emit idiomatic C++ rather than generic helper call soup.
6. Design closures around capture safety. Drast can become much more expressive for math, science, and data workflows if local function values lower to clean C++ lambdas with safe capture defaults.
7. Treat generated C++ as a public artifact. The output should be code a strong C++ developer would trust in a review: clear types, no avoidable allocations, no accidental raw pointers, no broad catches, no missing `override`, and no template placeholders leaking into user code.
