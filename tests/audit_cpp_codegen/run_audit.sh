#!/usr/bin/env bash
set -uo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/../.." && pwd)"
generated_dir="$script_dir/generated"
build_dir="$script_dir/build"

rm -rf "$generated_dir" "$build_dir"
mkdir -p "$generated_dir" "$build_dir"

failures=0

while IFS= read -r source; do
    case_name="${source#$script_dir/cases/}"
    case_name="${case_name%/main.drast}"
    case_slug="${case_name//\//__}"
    cpp_out="$generated_dir/$case_slug"
    exe_out="$build_dir/$case_slug"

    echo "== $case_name =="
    mkdir -p "$cpp_out" "$exe_out"
    if ! transpiler transpile "$source" -o "$cpp_out" --no-auto-discover; then
        echo "transpile failed: $case_name" >&2
        failures=$((failures + 1))
        continue
    fi

    if ! transpiler build "$source" -o "$exe_out" --no-auto-discover >"$exe_out/build.stdout" 2>"$exe_out/build.stderr"; then
        echo "build failed: $case_name" >&2
        failures=$((failures + 1))
    fi
done < <(find "$script_dir/cases" -name main.drast | sort)

if [[ $failures -gt 0 ]]; then
    echo "$failures audit case(s) failed to build" >&2
    exit 1
fi
