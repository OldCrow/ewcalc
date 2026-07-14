#!/usr/bin/env bash
# scripts/lint-cpp.sh
# Static analysis for the platform-agnostic core (libew, ewpresenter, bridge):
#   1. clang-tidy — bug-finding checks, configured via .clang-tidy (repo root)
#   2. cppcheck   — warning/style/performance/portability checks
#
# Mirrors the "static-analysis" job in .github/workflows/ci.yml so the same
# checks can be run locally before pushing. Frontend code (macOS/Linux/Windows)
# is out of scope here — see AGENTS.md Coding Conventions for per-frontend
# tooling (scripts/lint-linux.sh covers the Qt6 frontend).
#
# Usage:
#   ./scripts/lint-cpp.sh
#
# Prerequisites:
#   - clang-tidy and cppcheck on PATH

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$REPO_ROOT/build/lint"

# Unset any Homebrew LLVM overrides so compile_commands.json reflects the same
# AppleClang ABI the rest of the project builds with (see AGENTS.md Session
# Start). clang-tidy itself is a separate analysis tool, unaffected by this.
unset CC CXX CPP LD AR RANLIB STRIP NM OBJDUMP
unset CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_AR CMAKE_RANLIB
unset CPPFLAGS CXXFLAGS LDFLAGS

echo ""
echo "==> Configuring (compile_commands.json)..."

cmake -B "$BUILD_DIR" \
    -DEWCALC_BUILD_TESTS=OFF \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

echo ""
echo "==> Running clang-tidy (libew/src, ewpresenter/src, bridge)..."

# Portable file collection (avoids mapfile for bash 3.2 compatibility).
CPP_FILES=()
while IFS= read -r -d '' f; do
    CPP_FILES+=("$f")
done < <(find "$REPO_ROOT/libew/src" "$REPO_ROOT/ewpresenter/src" "$REPO_ROOT/bridge" \
    -maxdepth 1 -name '*.cpp' -print0)

clang-tidy -p "$BUILD_DIR" "${CPP_FILES[@]}"

echo ""
echo "==> Running cppcheck (libew, ewpresenter, bridge)..."

cppcheck --enable=warning,style,performance,portability \
    --std=c++20 --language=c++ --inline-suppr --error-exitcode=1 \
    -I "$REPO_ROOT/libew/include" -I "$REPO_ROOT/ewpresenter/include" -I "$REPO_ROOT/bridge" \
    "$REPO_ROOT/libew/src" "$REPO_ROOT/ewpresenter/src" "$REPO_ROOT/bridge/ewcalc_bridge.cpp"

echo ""
echo "==> Static analysis clean."
