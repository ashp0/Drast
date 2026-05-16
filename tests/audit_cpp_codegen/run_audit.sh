#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/../.." && pwd)"
compiler="${1:-$repo_root/build/bin/drast}"
if [[ "$compiler" != /* ]]; then
    compiler="$repo_root/$compiler"
fi
generated_dir="$script_dir/generated"
build_dir="$script_dir/build"

chmod -R u+w "$generated_dir" "$build_dir" 2>/dev/null || true
rm -rf "$generated_dir" "$build_dir"
mkdir -p "$generated_dir" "$build_dir"

failures=0

while IFS= read -r source; do
    case_name="${source#$script_dir/cases/}"
    case_name="${case_name%/main.drast}"
    case_slug="${case_name//\//__}"
    snapshot_dir="$generated_dir/$case_slug"
    project_dir="$build_dir/$case_slug/project"
    stdout="$build_dir/$case_slug/build.stdout"
    stderr="$build_dir/$case_slug/build.stderr"

    echo "== $case_name =="
    mkdir -p "$snapshot_dir" "$project_dir"
    cat >"$project_dir/package.txt" <<PKG
package audit_$case_slug
version 0.0.0
default case

target case
	kind binary
	entry $source
	output bin/case
	generated generated
	include $repo_root
	include $(dirname "$source")
	cxx c++17
PKG

    if ! (cd "$project_dir" && DRAST_HOME="$repo_root" "$compiler" build case >"$stdout" 2>"$stderr"); then
        echo "build failed: $case_name" >&2
        sed 's/^/  stderr: /' "$stderr" >&2
        failures=$((failures + 1))
        continue
    fi

    find "$project_dir/build/generated" -maxdepth 4 -name '*.cpp' -print0 |
        while IFS= read -r -d '' cpp; do
            cp "$cpp" "$snapshot_dir/$(basename "$cpp")"
        done
    cp "$project_dir/build/xmake/case/xmake.lua" "$snapshot_dir/xmake.lua"
done < <(find "$script_dir/cases" -name main.drast | sort)

if [[ $failures -gt 0 ]]; then
    echo "$failures audit case(s) failed to build" >&2
    exit 1
fi
