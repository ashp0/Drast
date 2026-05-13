#!/usr/bin/env bash
set -euo pipefail

compiler="${1:?usage: run_tests.sh <compiler> <repo-root>}"
repo_root="${2:?usage: run_tests.sh <compiler> <repo-root>}"
cxx="${CXX:-clang++}"
work_dir="${TMPDIR:-/tmp}/drast-tests-$$"

cleanup() {
    rm -rf "$work_dir"
}
trap cleanup EXIT

mkdir -p "$work_dir"

pass_count=0

run_case() {
    local name="$1"
    local source="$2"
    local expected="$3"
    local case_dir
    local cpp
    local bin
    local actual

    case_dir="$(dirname "$source")"
    cpp="$work_dir/$name.cpp"
    bin="$work_dir/$name"

    "$compiler" "$source" -o "$cpp"
    "$cxx" -std=c++17 -I"$repo_root" -I"$case_dir" "$cpp" -o "$bin"
    actual="$("$bin")"
    if [[ "$actual" != "$expected" ]]; then
        echo "FAIL $name: expected [$expected], got [$actual]" >&2
        exit 1
    fi
    echo "PASS $name"
    pass_count=$((pass_count + 1))
}

run_failure() {
    local name="$1"
    local source="$2"
    local expected_error="$3"
    local cpp="$work_dir/$name.cpp"
    local stdout="$work_dir/$name.out"
    local stderr="$work_dir/$name.err"

    if "$compiler" "$source" -o "$cpp" >"$stdout" 2>"$stderr"; then
        echo "FAIL $name: command unexpectedly succeeded" >&2
        exit 1
    fi
    if ! grep -q "$expected_error" "$stderr"; then
        echo "FAIL $name: expected error containing [$expected_error]" >&2
        cat "$stderr" >&2
        exit 1
    fi
    echo "PASS $name"
    pass_count=$((pass_count + 1))
}

run_case "single-use" "$repo_root/tests/use_cases/single/main.drast" "single-ok"
run_case "multiple-use" "$repo_root/tests/use_cases/multiple/main.drast" "multi-a:multi-b"
run_case "nested-use" "$repo_root/tests/use_cases/nested/main.drast" "nested-ok"
run_case "subdir-use" "$repo_root/tests/use_cases/subdir/main.drast" "42"
run_case "header-use" "$repo_root/tests/use_cases/header/main.drast" "42"
run_case "generic-struct" "$repo_root/tests/use_cases/generic/main.drast" "41"
run_case "pointer-deref" "$repo_root/tests/use_cases/pointers/main.drast" "25"
run_case "null-char" "$repo_root/tests/use_cases/null_char/main.drast" "0"

absolute_main="$work_dir/absolute_main.drast"
cat >"$absolute_main" <<ABSOLUTE_MAIN
use std
use '$repo_root/tests/use_cases/single/helper.drast'

main, int
	println greeting
	return 0
ABSOLUTE_MAIN
run_case "absolute-use" "$absolute_main" "single-ok"

run_failure "circular-use" "$repo_root/tests/use_cases/circular/main.drast" "circular import"

echo "All $pass_count tests passed"
