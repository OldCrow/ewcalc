#!/usr/bin/env bash
# scripts/lint-linux.sh
# Static analysis for the Linux Qt6 frontend (frontend/linux):
#   cppcheck — warning/style/performance/portability checks
#
# Qt's macro-heavy style (Q_OBJECT, signal/slot syntax) does not require any
# suppressions or a Qt-aware ruleset — verified clean against this codebase.
# No clang-tidy here: Qt's moc-generated code and CMake's Qt6 package config
# would need a full Qt-aware compile_commands.json, which the frontend build
# does not currently export; cppcheck alone catches genuine issues without it.
#
# Gated with --error-exitcode=1, matching scripts/lint-cpp.sh.
#
# Usage:
#   ./scripts/lint-linux.sh
#
# Prerequisites:
#   - cppcheck on PATH

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo ""
echo "==> Running cppcheck (frontend/linux)..."

cppcheck --enable=warning,style,performance,portability \
    --std=c++20 --language=c++ --inline-suppr --error-exitcode=1 \
    -I "$REPO_ROOT/libew/include" -I "$REPO_ROOT/ewpresenter/include" \
    -I "$REPO_ROOT/frontend/linux/src" \
    "$REPO_ROOT/frontend/linux/src"

echo ""
echo "==> cppcheck complete."
