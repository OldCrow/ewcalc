cmake_minimum_required(VERSION 3.20)
project(ewcalc-macos LANGUAGES CXX Swift)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_Swift_LANGUAGE_VERSION "5")
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "")

# ── Pre-built native library locations ────────────────────────────────────────
# Built by the parent cmake invocation before this project is configured.
# On macOS with single-config Makefiles/Ninja, CMake prefixes target 'libew'
# with 'lib' → liblibew.a (same naming convention as the Linux build).
if(NOT DEFINED EWCALC_NATIVE_BUILD_DIR)
    get_filename_component(EWCALC_NATIVE_BUILD_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/../../build" ABSOLUTE)
endif()
get_filename_component(EWCALC_SRC_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

foreach(_lib liblibew.a libewpresenter.a)
    if(NOT EXISTS "${EWCALC_NATIVE_BUILD_DIR}/lib/${_lib}")
        message(FATAL_ERROR
            "${_lib} not found at ${EWCALC_NATIVE_BUILD_DIR}/lib/\n"
            "Build native libs first:\n"
            "  cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build")
    endif()
endforeach()

add_library(libew_native STATIC IMPORTED)
set_target_properties(libew_native PROPERTIES
    IMPORTED_LOCATION "${EWCALC_NATIVE_BUILD_DIR}/lib/liblibew.a"
    INTERFACE_INCLUDE_DIRECTORIES "${EWCALC_SRC_ROOT}/libew/include"
)

add_library(ewpresenter_native STATIC IMPORTED)
set_target_properties(ewpresenter_native PROPERTIES
    IMPORTED_LOCATION "${EWCALC_NATIVE_BUILD_DIR}/lib/libewpresenter.a"
    INTERFACE_INCLUDE_DIRECTORIES "${EWCALC_SRC_ROOT}/ewpresenter/include"
)

# ── Signing identity ─────────────────────────────────────────────────────────
# Override on the cmake command line for distribution builds:
#   -DEWCALC_CODESIGN_IDENTITY="Developer ID Application: Gary Wolfman (6HGS466D28)"
#   -DEWCALC_TEAM_ID="6HGS466D28"
if(NOT DEFINED EWCALC_CODESIGN_IDENTITY)
    set(EWCALC_CODESIGN_IDENTITY "-")
endif()
if(NOT DEFINED EWCALC_TEAM_ID)
    set(EWCALC_TEAM_ID "")
endif()

# ── C bridge (pre-built by native cmake, IMPORTED here) ────────────────────────
# ewcalc_bridge is compiled by the native cmake step (Makefiles/Ninja) with
# CMAKE_OSX_DEPLOYMENT_TARGET=13.0, avoiding the Xcode generator's failure
# to propagate that setting for C++ static library targets.
foreach(_lib libewcalc_bridge.a)
    if(NOT EXISTS "${EWCALC_NATIVE_BUILD_DIR}/lib/${_lib}")
        message(FATAL_ERROR
            "${_lib} not found at ${EWCALC_NATIVE_BUILD_DIR}/lib/\n"
            "Run: cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=... "
            "&& cmake --build build")
    endif()
endforeach()

add_library(ewcalc_bridge STATIC IMPORTED)
set_target_properties(ewcalc_bridge PROPERTIES
    IMPORTED_LOCATION "${EWCALC_NATIVE_BUILD_DIR}/lib/libewcalc_bridge.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/bridge"
)

# ── Swift app bundle ──────────────────────────────────────────────────────────
set(SWIFT_SOURCES
    app/ewcalcApp.swift
    app/ContentView.swift
    app/Adapters/EwCalcStore.swift
    app/Adapters/PropagationAdapter.swift
    app/Adapters/LinkAdapter.swift
    app/Adapters/ReceiverAdapter.swift
    app/Adapters/JammingAdapter.swift
    app/Adapters/LocationAdapter.swift
    app/Adapters/RadarAdapter.swift
    app/Adapters/DigitalAdapter.swift
    app/Adapters/AntennaAdapter.swift
    app/Views/Shared.swift
    app/Views/PropagationView.swift
    app/Views/LinkView.swift
    app/Views/ReceiverView.swift
    app/Views/JammingView.swift
    app/Views/LocationView.swift
    app/Views/RadarView.swift
    app/Views/DigitalView.swift
    app/Views/AntennaView.swift
    app/Views/ReferenceView.swift
)

# App icon (must be a resource inside the bundle)
set(ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/app/Assets/AppIcon.icns")
set_source_files_properties(${ICON_FILE} PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)

add_executable(ewcalc MACOSX_BUNDLE ${SWIFT_SOURCES} ${ICON_FILE})

# libc++ must be explicitly listed: the bridge is a pre-built IMPORTED
# static library; Xcode's Swift target does not automatically add C++
# runtime linkage for imported libraries the way it does for source targets.
# IMPORTED static libraries do not carry transitive link deps; list all
# libraries that ewcalc_bridge depends on explicitly.
target_link_libraries(ewcalc PRIVATE ewcalc_bridge ewpresenter_native libew_native c++)

# Include bridge/ so the bridging header can resolve #include "ewcalc_bridge.h"
target_include_directories(ewcalc PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/bridge")

set_target_properties(ewcalc PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST         "${CMAKE_CURRENT_SOURCE_DIR}/app/Info.plist"
    MACOSX_BUNDLE_ICON_FILE          "AppIcon.icns"
    MACOSX_BUNDLE_BUNDLE_IDENTIFIER  "com.oldcrow.ewcalc"
    MACOSX_BUNDLE_BUNDLE_NAME        "EW Calculator"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.6.5"
    MACOSX_BUNDLE_BUNDLE_VERSION     "1"

    # Xcode build settings (only effective with -G Xcode)
    XCODE_ATTRIBUTE_SWIFT_VERSION                         "5.9"
    XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER
        "${CMAKE_CURRENT_SOURCE_DIR}/app/BridgingHeader.h"
    XCODE_ATTRIBUTE_SWIFT_INSTALL_OBJC_HEADER             "NO"
    XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET              "13.0"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER             "com.oldcrow.ewcalc"
    XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD          "c++20"
    XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME               "YES"
    # Pass -DEWCALC_CODESIGN_IDENTITY="Developer ID Application: ..." for
    # distribution builds. Defaults to ad-hoc ("-") for CI / local dev.
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY                    "${EWCALC_CODESIGN_IDENTITY}"
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM                      "${EWCALC_TEAM_ID}"
    XCODE_ATTRIBUTE_CODE_SIGN_STYLE                       "Manual"
    XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED                 "YES"
    # Notarization requires a secure timestamp in the signature.
    XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS                 "--timestamp"
    # Prevent Xcode injecting the get-task-allow debug entitlement;
    # notarization rejects any binary that carries it.
    XCODE_ATTRIBUTE_CODE_SIGN_INJECT_BASE_ENTITLEMENTS    "NO"
    XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS               "@executable_path/../Frameworks"
)
