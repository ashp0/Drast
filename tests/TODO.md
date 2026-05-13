# Drast Test Suite TODO

## Baseline Run

- Pre-existing xmake suite before adding new tests: 10 passed, 0 failed.
- New default suite, excluding `tests/pending/`: 118 passed, 10 failed.
- New suite with `--include-pending`: 124 passed, 10 failed.
- Pending `nothing` tests are intentionally isolated under `tests/pending/`.

## BUG-001: Nested array literals are parsed as indexing

**Severity:** Medium
**Discovered by:** Test file `tests/collections/nested_arrays.drast`
**Status:** Open

### Description
The valid nested collection literal below is rejected:

```drast
matrix = {{1 2} {3 4}}
```

### Expected behaviour
According to `SYNTAX.md`, `{expr ...}` is an array literal, so inner array literals should be accepted as element expressions for nested collections.

### Actual behaviour
The compiler exits with:

```text
expected index close
expected end of statement
```

### Pinpointed cause (if known)
Likely `src/features/expressions.drast`, around `parsePostfixNoImplicit`: after parsing the first `{1 2}` as an expression, the next `{3 4}` is treated as postfix indexing on that first expression instead of as the next array element.

### Proposed fix (if known)
When parsing array literal elements, avoid letting a following `{...}` become postfix indexing on the previous element unless the grammar has explicit indexing context. A small parser context flag for “inside array literal item list” may be enough.

## BUG-002: Undeclared identifiers are accepted

**Severity:** High
**Discovered by:** Test file `tests/errors/undeclared_variable.drast`
**Status:** Open

### Description
The compiler accepts:

```drast
main, int
	return missingValue
```

### Expected behaviour
The transpiler should reject use of an undeclared variable before emitting C++.

### Actual behaviour
The transpiler exits successfully and emits a reference to `missingValue`, leaving the error for the downstream C++ compiler.

### Pinpointed cause (if known)
Likely `src/features/expressions.drast`, around `parsePrimary` and `lookupNameType`: identifiers are emitted even when `lookupNameType` returns an empty type and the name is not a known function, method, field, enum variant, type, or standard helper.

### Proposed fix (if known)
Track declared locals/globals/functions/types and report an `undeclared identifier` diagnostic when a normal identifier cannot be resolved.

## BUG-003: Local use-before-declaration is accepted

**Severity:** Medium
**Discovered by:** Test file `tests/errors/use_before_declaration.drast`
**Status:** Open

### Description
The compiler accepts:

```drast
main, int
	value = later
	later = 1
	return value
```

### Expected behaviour
Local variables should not be usable before their declaration or first assignment.

### Actual behaviour
The transpiler exits successfully and emits C++ that references `later` before it exists.

### Pinpointed cause (if known)
Same area as BUG-002: `lookupNameType` returns empty for unknown locals, but the parser does not emit a diagnostic before generating identifier code.

### Proposed fix (if known)
Add local declaration/definite-assignment checks before accepting an identifier expression in function bodies.

## BUG-004: Assignment type mismatches are accepted

**Severity:** High
**Discovered by:** Test file `tests/types/type_mismatch_assignment_rejected.drast`
**Status:** Open

### Description
The compiler accepts typed declarations and assignments with incompatible values, for example:

```drast
count int = s'not an int'
```

Related test: `tests/errors/type_mismatch_assignment.drast`.

### Expected behaviour
The transpiler should reject assigning a `string` expression to an `int` variable.

### Actual behaviour
The transpiler exits successfully and emits invalid C++.

### Pinpointed cause (if known)
Likely `src/features/variables.drast`, around `parseSimpleStatement`: typed declarations record the declared type in `localTypes`, but they do not compare it with the initializer or assigned expression type.

### Proposed fix (if known)
When both sides have known non-empty types, compare them during declarations and assignments. Allow documented conversions only through explicit casts such as `Int[...]`, `Float[...]`, etc.

## BUG-005: Function call argument type mismatches are accepted

**Severity:** High
**Discovered by:** Test file `tests/errors/type_mismatch_function_call.drast`
**Status:** Open

### Description
The compiler accepts:

```drast
add left;int right;int, int
	return left + right

main, int
	return add s'one' 2
```

### Expected behaviour
The argument `s'one'` should be rejected because `add` expects an `int`.

### Actual behaviour
The transpiler exits successfully and emits the call.

### Pinpointed cause (if known)
`src/features/function_declaration.drast` stores return types in `functionReturns`, but there does not appear to be an equivalent parameter-signature table used by `parseImplicitCall`.

### Proposed fix (if known)
Store function parameter types during declaration/predeclaration and validate argument expressions in `parseImplicitCall` / `callCode`.

## BUG-006: Wrong argument counts are accepted

**Severity:** High
**Discovered by:** Test file `tests/errors/wrong_argument_count.drast`
**Status:** Open

### Description
The compiler accepts a call with too few arguments:

```drast
return add 1
```

### Expected behaviour
The transpiler should reject calls whose arity does not match the function signature, accounting for default and variadic parameters.

### Actual behaviour
The transpiler exits successfully and emits the bad call.

### Pinpointed cause (if known)
Same missing signature-validation path as BUG-005.

### Proposed fix (if known)
Validate call arity against stored function/method signatures before code emission.

## BUG-007: Non-function values are callable

**Severity:** High
**Discovered by:** Test file `tests/errors/call_non_function.drast`
**Status:** Open

### Description
The compiler accepts:

```drast
value = 1
value 2
```

### Expected behaviour
Calling a non-function integer value should be rejected.

### Actual behaviour
The transpiler exits successfully and emits `value(2);`.

### Pinpointed cause (if known)
Likely `src/features/expressions.drast`, around `isCallee`: any identifier or field access can become a callee without checking whether it resolves to a function/method/type.

### Proposed fix (if known)
Restrict implicit calls to known callable symbols, type constructors, and supported runtime helpers.

## BUG-008: Duplicate variable declarations in one scope are accepted

**Severity:** Medium
**Discovered by:** Test file `tests/errors/duplicate_variable_same_scope.drast`
**Status:** Open

### Description
The compiler accepts:

```drast
value int = 1
value int = 2
```

### Expected behaviour
A duplicate typed declaration in the same local scope should be rejected.

### Actual behaviour
The transpiler exits successfully and emits two C++ declarations with the same name.

### Pinpointed cause (if known)
Likely `src/features/variables.drast`, around typed declaration handling: `localTypes.set name t.text` overwrites any existing entry without reporting a duplicate.

### Proposed fix (if known)
Before adding a typed local declaration, check whether the name already exists in the current scope and emit a duplicate declaration diagnostic.

## BUG-009: Missing returns in non-void functions are accepted

**Severity:** Medium
**Discovered by:** Test file `tests/errors/missing_return_statement.drast`
**Status:** Open

### Description
The compiler accepts:

```drast
main, int
	value = 1
```

### Expected behaviour
A non-void function should require a return on all paths, or at minimum a final return statement if full control-flow analysis is not implemented.

### Actual behaviour
The transpiler exits successfully and emits an `int main` body without a return.

### Pinpointed cause (if known)
`src/features/function_declaration.drast` parses and emits a body but does not validate return coverage for non-void functions.

### Proposed fix (if known)
Start with a conservative check requiring an explicit `return` in non-void function bodies, then improve to path-sensitive control-flow analysis later.
