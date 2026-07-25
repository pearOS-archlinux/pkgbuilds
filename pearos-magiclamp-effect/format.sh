#!/usr/bin/env bash
set -euo pipefail

base="$(cd "$(dirname "$0")" && pwd)"

if ! command -v clang-format &>/dev/null; then
    echo "clang-format not found. Install it first (e.g. sudo pacman -S clang)."
    exit 1
fi

find "$base/src" -name '*.cc' -o -name '*.h' | xargs clang-format -i
echo "Done."
