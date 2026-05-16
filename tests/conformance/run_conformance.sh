#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/../.." && pwd)"
compiler="${DRASTC:-$repo_root/build/bin/drast}"

if [[ ! -x "$compiler" ]]; then
    echo "FAIL: compiler not found: $compiler" >&2
    exit 2
fi

work_dir="$(mktemp -d "${TMPDIR:-/tmp}/drast-conformance.XXXXXX")"
cleanup() {
    rm -rf "$work_dir"
}
trap cleanup EXIT

cat >"$work_dir/package.txt" <<PKG
package conformance
version 0.0.0
default harness

target harness
	kind binary
	entry $script_dir/harness.drast
	include $repo_root
	cxx c++17
PKG

(cd "$work_dir" && DRAST_HOME="$repo_root" DRAST_TYPECHECK=0 "$compiler" build harness >/dev/null)

harness="$work_dir/build/bin/harness"
if [[ ! -x "$harness" ]]; then
    echo "FAIL: conformance harness did not build" >&2
    exit 2
fi

passed=0
failed=0

for source in "$script_dir"/E*.must-accept.drast "$script_dir"/E*.must-reject.drast; do
    [[ -e "$source" ]] || continue
    base="$(basename "$source")"
    expectation="accept"
    expected_code=""
    if [[ "$base" == *.must-reject.drast ]]; then
        expectation="reject"
        expected_code="${base%%-*}"
    fi
    if "$harness" "$source" "$expectation" "$expected_code" >"$work_dir/$base.out" 2>&1; then
        echo "PASS conformance/$base"
        passed=$((passed + 1))
    else
        echo "FAIL conformance/$base"
        sed 's/^/  /' "$work_dir/$base.out"
        failed=$((failed + 1))
    fi
done

echo "$passed conformance passed, $failed failed"
if [[ $failed -ne 0 ]]; then
    exit 1
fi
