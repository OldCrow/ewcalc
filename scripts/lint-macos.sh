#!/usr/bin/env bash
# scripts/lint-macos.sh
# Static analysis for the macOS SwiftUI frontend (frontend/macos):
#   SwiftLint — style/convention checks, configured via .swiftlint.yml (repo root)
#
# Matches the scripts/lint-cpp.sh / scripts/lint-linux.sh pattern: a
# standalone script runnable locally, independent of the Xcode build.
#
# Usage:
#   ./scripts/lint-macos.sh
#
# Prerequisites:
#   - swiftlint on PATH (brew install swiftlint)

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo ""
echo "==> Running swiftlint (frontend/macos/app)..."

swiftlint lint --strict --config "$REPO_ROOT/.swiftlint.yml" "$REPO_ROOT/frontend/macos/app"

echo ""
echo "==> swiftlint complete."
