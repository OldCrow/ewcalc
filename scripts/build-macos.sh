#!/usr/bin/env bash
# scripts/build-macos.sh
# Builds the full macOS ewcalc stack:
#   1. CMake: libew + ewpresenter (native C++ static libs)
#   2. xcodebuild: SwiftUI/AppKit frontend (Phase 4 — not yet implemented)
#   3. Optionally packages as .pkg or .dmg (pass --package)
#
# Usage:
#   ./scripts/build-macos.sh                  # build only
#   ./scripts/build-macos.sh --config Debug   # debug build
#   ./scripts/build-macos.sh --package        # build + create .pkg installer
#
# Prerequisites:
#   - Xcode 15+ with Command Line Tools
#   - CMake 3.20+
#   - (For distribution) Apple Developer ID certificate in keychain

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CONFIG="Release"
PACKAGE=0
ARCH="$(uname -m)"   # arm64 on Apple Silicon, x86_64 on Intel

# ── Argument parsing ─────────────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --config) CONFIG="$2"; shift 2 ;;
        --package) PACKAGE=1; shift ;;
        *) echo "Unknown argument: $1"; exit 1 ;;
    esac
done

BUILD_DIR="$REPO_ROOT/build"
MSIX_DIR="$BUILD_DIR/pkg"

echo ""
echo "==> Building native libraries (CMake)..."

cmake -B "$BUILD_DIR" \
    -DEWCALC_BUILD_TESTS=OFF \
    -DCMAKE_BUILD_TYPE="$CONFIG" \
    -DCMAKE_OSX_ARCHITECTURES="$ARCH"

cmake --build "$BUILD_DIR" --config "$CONFIG" --parallel "$(sysctl -n hw.logicalcpu)"

echo "    Native libs built to: $BUILD_DIR/lib/$CONFIG/"

# ── macOS frontend (Phase 4 placeholder) ─────────────────────────────────────
XCODEPROJ="$REPO_ROOT/frontend/macos/ewcalc.xcodeproj"

if [[ -d "$XCODEPROJ" ]]; then
    echo ""
    echo "==> Building macOS frontend (xcodebuild)..."

    xcodebuild \
        -project "$XCODEPROJ" \
        -scheme "ewcalc" \
        -configuration "$CONFIG" \
        -arch "$ARCH" \
        SYMROOT="$BUILD_DIR/macos" \
        build

    APP_PATH="$BUILD_DIR/macos/$CONFIG/ewcalc.app"
    echo "    App built to: $APP_PATH"

    if [[ $PACKAGE -eq 1 ]]; then
        echo ""
        echo "==> Packaging as .pkg..."

        mkdir -p "$MSIX_DIR"
        PKG_OUTPUT="$MSIX_DIR/ewcalc-$ARCH-$CONFIG.pkg"

        # productbuild creates a distributable installer package
        productbuild \
            --component "$APP_PATH" /Applications \
            "$PKG_OUTPUT"

        echo "    Package: $PKG_OUTPUT"

        # To also create a .dmg:
        # hdiutil create -volname "ewcalc" -srcfolder "$APP_PATH" \
        #     -ov -format UDZO "$MSIX_DIR/ewcalc-$ARCH-$CONFIG.dmg"
    fi
else
    echo ""
    echo "    [SKIP] macOS frontend not yet implemented (Phase 4)."
    echo "           Expected Xcode project at: $XCODEPROJ"
fi

echo ""
echo "==> Build complete."
