#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_DIR="${SCRIPT_DIR}/pearos-about"
BUILD_DIR="${APP_DIR}/build"

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake .. "${@}"
make -j"$(nproc)"

echo
echo "Binar construit:"
echo "  ${BUILD_DIR}/pearos-about-qt"

