#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"

BUILD_DIR="build"
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build "$BUILD_DIR" --parallel

echo ""
echo "Build done: ./$BUILD_DIR/pearos-notch"
echo "Run with:   ./$BUILD_DIR/pearos-notch"
