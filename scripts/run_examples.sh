#!/bin/bash
set -e

COMPILER="$1"
EXAMPLES_DIR="$2"

if [ -z "$COMPILER" ] || [ -z "$EXAMPLES_DIR" ]; then
    echo "Usage: $0 <compiler> <examples_dir>"
    exit 1
fi

find "$EXAMPLES_DIR" -name "*.drast" -type f | sort | while read example; do
    echo "=== $example ==="
    "$COMPILER" "$example"
done
