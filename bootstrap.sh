#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
legacy_seed="$repo_root/bootstrap/legacy/darwin-arm64/drastc"
seed="$legacy_seed"
seed_mode="legacy"
check=0

echo "drast runtime will be removed, it is deprecated. wait a while for the other agent to fix it."
echo "When I meant we should remove drast_runtime.hpp I did mean it. But I really meant that we should rely solely on Drast rather than still having a hidden C++ layer within which I do not like."


usage() {
    cat <<USAGE
usage: ./bootstrap.sh [--check] [--seed <legacy-binary>] [--from-new <drast-binary>]

  --check       run the self-host validation path
  --seed        use a legacy compiler that supports "source -o cpp"
  --from-new    use a package-era Drast compiler that supports "drast build"
USAGE
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --check)
            check=1
            shift
            ;;
        --seed)
            seed="${2:?--seed requires a path}"
            seed_mode="legacy"
            shift 2
            ;;
        --from-new)
            seed="${2:?--from-new requires a path}"
            seed_mode="new"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            usage >&2
            exit 2
            ;;
    esac
done

if [[ "$seed" != /* ]]; then
    seed="$repo_root/$seed"
fi

verify_hash() {
    local file="$1"
    local hash_file="$2"
    if [[ ! -f "$hash_file" ]]; then
        return 0
    fi
    local expected
    local actual
    expected="$(awk '{print $1}' "$hash_file")"
    actual="$(shasum -a 256 "$file" | awk '{print $1}')"
    if [[ "$expected" != "$actual" ]]; then
        echo "bootstrap hash mismatch for $file" >&2
        echo "expected $expected" >&2
        echo "actual   $actual" >&2
        exit 1
    fi
}

require_tool() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "required tool not found: $1" >&2
        exit 1
    fi
}

require_tool shasum
require_tool clang++
require_tool xmake

if [[ ! -x "$seed" ]]; then
    echo "seed compiler is missing or not executable: $seed" >&2
    exit 1
fi

case "$seed_mode" in
    legacy)
        verify_hash "$seed" "$seed.sha256"
        if [[ "$seed" == "$legacy_seed" ]]; then
            verify_hash "$seed" "$repo_root/bootstrap/legacy/darwin-arm64/drastc.sha256"
        fi
        stage_dir="$repo_root/build/bootstrap/stage0"
        stage_cpp="$stage_dir/drast.cpp"
        stage_bin="$stage_dir/drast"
        mkdir -p "$stage_dir"
        chmod u+w "$stage_cpp" 2>/dev/null || true
        "$seed" "$repo_root/src/main.drast" -o "$stage_cpp"
        clang++ -std=c++17 -I"$repo_root" "$stage_cpp" -o "$stage_bin"
        seed="$stage_bin"
        ;;
    new)
        verify_hash "$seed" "$seed.sha256"
        stage_dir="$repo_root/build/bootstrap/new-seed"
        staged_seed="$stage_dir/drast"
        mkdir -p "$stage_dir"
        cp "$seed" "$staged_seed"
        chmod +x "$staged_seed"
        seed="$staged_seed"
        ;;
esac

(cd "$repo_root" && DRAST_HOME="$repo_root" "$seed" build drast)

built="$repo_root/build/bin/drast"
if [[ ! -x "$built" ]]; then
	echo "bootstrap build did not produce $built" >&2
	exit 1
fi

if [[ $check -eq 1 ]]; then
    (cd "$repo_root" && DRAST_HOME="$repo_root" "$built" build drast)
    "$built" help >/dev/null
fi

echo "bootstrap ok: $built"
