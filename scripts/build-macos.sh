#!/usr/bin/env bash
# scripts/build-macos.sh
# Builds the full macOS ewcalc stack:
#   1. CMake: libew + ewpresenter (native C++ static libs)
#   2. cmake -G Xcode: SwiftUI frontend (Phase 4)
#   3. Optionally packages as .pkg (pass --package; deferred until signing is ready)
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
PKG_DIR="$BUILD_DIR/pkg"

echo ""
echo "==> Building native libraries (CMake)..."

cmake -B "$BUILD_DIR" \
    -DEWCALC_BUILD_TESTS=OFF \
    -DCMAKE_BUILD_TYPE="$CONFIG" \
    -DCMAKE_OSX_ARCHITECTURES="$ARCH"

cmake --build "$BUILD_DIR" --config "$CONFIG" --parallel "$(sysctl -n hw.logicalcpu)"

echo "    Native libs built to: $BUILD_DIR/lib/"

# ── macOS frontend (Phase 4) ──────────────────────────────────────────────────
MACOS_FRONTEND="$REPO_ROOT/frontend/macos/CMakeLists.txt"

if [[ -f "$MACOS_FRONTEND" ]]; then
    echo ""
    echo "==> Building macOS frontend (cmake -G Xcode)..."

    FRONTEND_BUILD="$BUILD_DIR/macos-frontend"

    # Unset any LLVM-specific overrides so swiftc uses Apple's toolchain.
    # LD in particular causes Swift's linker test to fail if pointed at lld.
    unset LD CC CXX CPP AR RANLIB STRIP NM OBJDUMP
    unset CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_AR CMAKE_RANLIB
    unset CPPFLAGS CXXFLAGS LDFLAGS

    # Configure: generate an Xcode project that CMake manages
    cmake -G Xcode \
        -B "$FRONTEND_BUILD" \
        -S "$REPO_ROOT/frontend/macos" \
        -DEWCALC_NATIVE_BUILD_DIR="$BUILD_DIR" \
        -DCMAKE_OSX_ARCHITECTURES="$ARCH" \
        -DEWCALC_CODESIGN_IDENTITY="Developer ID Application: Gary Wolfman (6HGS466D28)" \
        -DEWCALC_TEAM_ID="6HGS466D28"

    # Build
    cmake --build "$FRONTEND_BUILD" --config "$CONFIG" -- \
        -jobs "$(sysctl -n hw.logicalcpu)"

    APP_PATH="$FRONTEND_BUILD/$CONFIG/ewcalc.app"
    echo "    App built to: $APP_PATH"

    if [[ $PACKAGE -eq 1 ]]; then
        echo ""
        echo "==> Notarizing and packaging..."

        # ── Notarize ─────────────────────────────────────────────────────
        # Requires credentials stored via:
        #   xcrun notarytool store-credentials ewcalc-notarytool \
        #       --apple-id <apple-id> --team-id 6HGS466D28 \
        #       --password <app-specific-password>
        echo "    Submitting for notarization (this may take several minutes)..."
        xcrun notarytool submit "$APP_PATH" \
            --keychain-profile "ewcalc-notarytool" \
            --wait

        echo "    Stapling notarization ticket..."
        xcrun stapler staple "$APP_PATH"

        # ── Package ──────────────────────────────────────────────────────
        mkdir -p "$PKG_DIR"
        PKG_OUTPUT="$PKG_DIR/ewcalc-$ARCH-$CONFIG.pkg"

        productbuild \
            --component "$APP_PATH" /Applications \
            --sign "Developer ID Installer: Gary Wolfman (6HGS466D28)" \
            "$PKG_OUTPUT"

        echo "    Package: $PKG_OUTPUT"
    fi
else
    echo ""
    echo "    [SKIP] macOS frontend not found."
    echo "           Expected CMakeLists.txt at: $MACOS_FRONTEND"
fi

echo ""
echo "==> Build complete."
