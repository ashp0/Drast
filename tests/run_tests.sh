#!/usr/bin/env bash
set -u -o pipefail

include_pending=0
compiler="${DRASTC:-}"
repo_root=""

usage() {
    echo "usage: tests/run_tests.sh [--include-pending] [compiler] [repo-root]" >&2
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --include-pending)
            include_pending=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            if [[ -z "$compiler" ]]; then
                compiler="$1"
            elif [[ -z "$repo_root" ]]; then
                repo_root="$1"
            else
                usage
                exit 2
            fi
            shift
            ;;
    esac
done

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ -z "$repo_root" ]]; then
    repo_root="$(cd "$script_dir/.." && pwd)"
fi

if [[ -z "$compiler" ]]; then
    for candidate in \
        "$repo_root/build/macosx/arm64/release/transpiler" \
        "$repo_root/build/selfhost-check/transpiler_selfhost" \
        "$repo_root/dist/drastc-darwin-arm64"; do
        if [[ -x "$candidate" ]]; then
            compiler="$candidate"
            break
        fi
    done
fi

if [[ -z "$compiler" || ! -x "$compiler" ]]; then
    echo "FAIL: could not find executable compiler; pass one explicitly or set DRASTC" >&2
    exit 2
fi

work_dir="$(mktemp -d "${TMPDIR:-/tmp}/drast-suite.XXXXXX")"
timeout_seconds="${DRAST_TEST_TIMEOUT:-20}"
cleanup() {
    rm -rf "$work_dir"
}
trap cleanup EXIT

passed=0
failed=0

test_files=()
while IFS= read -r file; do
    test_files+=("$file")
done < <(find "$script_dir" -type f -name '*.drast' | sort)

header_value() {
    local key="$1"
    local file="$2"
    sed -nE "s/^[[:space:]]*(\/\/[[:space:]]*)?# ${key}:[[:space:]]*//p" "$file" | head -n 1
}

for source in "${test_files[@]}"; do
    rel="${source#$script_dir/}"
    if [[ $include_pending -eq 0 && "$rel" == pending/* ]]; then
        continue
    fi

    expect="$(header_value EXPECT "$source")"
    error_contains="$(header_value ERROR_CONTAINS "$source")"

    if [[ -z "$expect" ]]; then
        echo "FAIL $rel: missing # EXPECT header"
        failed=$((failed + 1))
        continue
    fi

    safe_name="${rel//[^A-Za-z0-9_]/_}"
    cpp="$work_dir/$safe_name.cpp"
    stdout="$work_dir/$safe_name.out"
    stderr="$work_dir/$safe_name.err"

    timeout_marker="$work_dir/$safe_name.timeout"
    "$compiler" "$source" -o "$cpp" >"$stdout" 2>"$stderr" &
    pid=$!
    (
        sleep "$timeout_seconds"
        if kill -0 "$pid" 2>/dev/null; then
            echo timed-out >"$timeout_marker"
            kill "$pid" 2>/dev/null || true
            sleep 1
            kill -9 "$pid" 2>/dev/null || true
        fi
    ) &
    watchdog=$!
    wait "$pid" 2>/dev/null
    status=$?
    kill "$watchdog" 2>/dev/null || true
    wait "$watchdog" 2>/dev/null || true
    if [[ -f "$timeout_marker" ]]; then
        status=124
        echo "compiler timed out after ${timeout_seconds}s" >"$stderr"
    fi

    case "$expect" in
        pass)
            if [[ $status -eq 0 ]]; then
                echo "PASS $rel"
                passed=$((passed + 1))
            else
                echo "FAIL $rel: expected success, got exit $status"
                sed 's/^/  stderr: /' "$stderr"
                failed=$((failed + 1))
            fi
            ;;
        error)
            if [[ $status -eq 0 ]]; then
                echo "FAIL $rel: expected compiler error, got success"
                failed=$((failed + 1))
            elif [[ -n "$error_contains" ]] && ! grep -Fq "$error_contains" "$stderr"; then
                echo "FAIL $rel: expected stderr containing [$error_contains]"
                sed 's/^/  stderr: /' "$stderr"
                failed=$((failed + 1))
            else
                echo "PASS $rel"
                passed=$((passed + 1))
            fi
            ;;
        *)
            echo "FAIL $rel: unknown EXPECT value [$expect]"
            failed=$((failed + 1))
            ;;
    esac
done

echo "$passed passed, $failed failed"
if [[ $failed -eq 0 ]]; then
    exit 0
fi
exit 1
