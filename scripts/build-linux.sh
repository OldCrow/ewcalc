#!/usr/bin/env bash
# scripts/build-linux.sh
# Builds the full Linux ewcalc stack:
#   1. CMake: libew + ewpresenter (native C++ static libs)
#   2. CMake: Qt6 frontend (Phase 5)
#   3. Optionally packages as .deb, .rpm, or AppImage (pass --package)
#
# Usage:
#   ./scripts/build-linux.sh                  # build only
#   ./scripts/build-linux.sh --config Debug   # debug build
#   ./scripts/build-linux.sh --package deb    # build + create .deb
#   ./scripts/build-linux.sh --package rpm    # build + create .rpm
#   ./scripts/build-linux.sh --package appimage  # build + create AppImage

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CONFIG="Release"
PACKAGE_FORMAT=""

# ── Argument parsing ─────────────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --config) CONFIG="$2"; shift 2 ;;
        --package) PACKAGE_FORMAT="$2"; shift 2 ;;
        *) echo "Unknown argument: $1"; exit 1 ;;
    esac
done

BUILD_DIR="$REPO_ROOT/build"
NPROC="$(nproc 2>/dev/null || echo 4)"

echo ""
echo "==> Building native libraries (CMake)..."

cmake -B "$BUILD_DIR" \
    -DEWCALC_BUILD_TESTS=OFF \
    -DCMAKE_BUILD_TYPE="$CONFIG"

cmake --build "$BUILD_DIR" --config "$CONFIG" --parallel "$NPROC"

echo "    Native libs built to: $BUILD_DIR/lib/$CONFIG/"

# ── Linux frontend (Phase 5) ─────────────────────────────────────────────────
LINUX_FRONTEND="$REPO_ROOT/frontend/linux/CMakeLists.txt"

if [[ -f "$LINUX_FRONTEND" ]]; then
    echo ""
    echo "==> Building Linux frontend (Qt6/CMake)..."

    cmake -B "$BUILD_DIR/linux-frontend" \
        -S "$REPO_ROOT/frontend/linux" \
        -DCMAKE_BUILD_TYPE="$CONFIG" \
        -DEWCALC_NATIVE_BUILD_DIR="$BUILD_DIR"

    cmake --build "$BUILD_DIR/linux-frontend" --parallel "$NPROC"

    echo "    Frontend built to: $BUILD_DIR/linux-frontend/"

    if [[ -n "$PACKAGE_FORMAT" ]]; then
        echo ""
        echo "==> Packaging as .$PACKAGE_FORMAT..."

        mkdir -p "$BUILD_DIR/pkg"

        case "$PACKAGE_FORMAT" in
            deb|rpm)
                # CPack handles .deb and .rpm when configured in the frontend CMakeLists
                cmake --build "$BUILD_DIR/linux-frontend" --target package
                ;;
            appimage)
                # AppImage requires linuxdeploy + Qt plugin
                # https://github.com/linuxdeploy/linuxdeploy-plugin-qt
                if ! command -v linuxdeploy &>/dev/null; then
                    echo "ERROR: linuxdeploy not found — install from https://github.com/linuxdeploy/linuxdeploy"
                    exit 1
                fi
                APPDIR="$BUILD_DIR/AppDir"
                cmake --install "$BUILD_DIR/linux-frontend" --prefix "$APPDIR/usr"
                linuxdeploy --appdir "$APPDIR" --plugin qt --output appimage
                mv ewcalc*.AppImage "$BUILD_DIR/pkg/"
                ;;
            *)
                echo "Unknown package format: $PACKAGE_FORMAT (use deb, rpm, or appimage)"
                exit 1
                ;;
        esac

        echo "    Package output: $BUILD_DIR/pkg/"
    fi
else
    echo ""
    echo "    [SKIP] Linux frontend not yet implemented (Phase 5)."
    echo "           Expected CMakeLists.txt at: $LINUX_FRONTEND"
fi

echo ""
echo "==> Build complete."
