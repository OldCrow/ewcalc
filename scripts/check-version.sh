#!/usr/bin/env bash
# scripts/check-version.sh
# Fails if an artifact's version disagrees with the root CMakeLists.txt
# project(ewcalc VERSION ...), the single source of the app version.
#
# Always checks the WinUI Package.appxmanifest (MSBuild-owned, so it cannot
# read the CMake version and is the one copy still kept by hand). With an
# .app path, also checks the built macOS bundle's Info.plist — the v1.3.0 DMG
# shipped with neither version key, which nothing caught.
#
# Usage:
#   ./scripts/check-version.sh [path/to/ewcalc.app]

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
fail=0

version="$(sed -nE 's/^project\(ewcalc VERSION ([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' \
    "$REPO_ROOT/CMakeLists.txt")"
[[ -n "$version" ]] || { echo "error: no project(ewcalc VERSION ...) in CMakeLists.txt" >&2; exit 1; }
echo "root version: $version"

check() {  # check <what> <actual> <expected>
    if [[ "$2" == "$3" ]]; then
        echo "  ok    $1 = $2"
    else
        echo "  FAIL  $1 = '${2}' (expected '$3')" >&2
        fail=1
    fi
}

manifest="$REPO_ROOT/frontend/windows/ewcalc-winui/ewcalc-winui/Package.appxmanifest"
check "Package.appxmanifest Identity Version" \
    "$(sed -nE 's/^[[:space:]]*Version="([0-9.]+)".*/\1/p' "$manifest" | head -1)" \
    "$version.0"

if [[ $# -ge 1 ]]; then
    plist="$1/Contents/Info.plist"
    for key in CFBundleShortVersionString CFBundleVersion; do
        check "$key" "$(/usr/libexec/PlistBuddy -c "Print :$key" "$plist" 2>/dev/null || true)" "$version"
    done
fi

exit "$fail"
