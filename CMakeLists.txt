cmake_minimum_required(VERSION 3.25)
project(ewcalc VERSION 1.0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Platform detection
if(WIN32)
    message(STATUS "ewcalc: Platform = Windows")
elseif(APPLE)
    message(STATUS "ewcalc: Platform = macOS")
else()
    message(STATUS "ewcalc: Platform = Linux")
endif()

# ── Options ──────────────────────────────────────────────────────────────
option(EWCALC_BUILD_TESTS    "Build libew test suite"           ON)
option(EWCALC_BUILD_FRONTEND "Build platform GUI frontend"      OFF)
set(EWCALC_FRONTEND_CONFIG "Release" CACHE STRING "MSBuild configuration (Windows)")
option(EWCALC_BUILD_COVERAGE "Instrument libew/ewpresenter/tests for code coverage (GCC+gcov or Clang+llvm-cov)" OFF)

# ── Test support ───────────────────────────────────────────────────────────
if(EWCALC_BUILD_TESTS)
    enable_testing()
endif()

# ── Coverage instrumentation ──────────────────────────────────────────────
# Applied directory-wide (before add_subdirectory(libew/ewpresenter)) so every
# target built afterwards — libraries, harness, and tests — is instrumented.
# Flags are gated here rather than hardcoded in CI workflow YAML.
if(EWCALC_BUILD_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
        add_link_options(-fprofile-instr-generate -fcoverage-mapping)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(--coverage)
        add_link_options(--coverage)
    else()
        message(WARNING "ewcalc: EWCALC_BUILD_COVERAGE is only supported with GCC or Clang — ignoring on ${CMAKE_CXX_COMPILER_ID}.")
    endif()
endif()

# ── Core libraries (always built) ────────────────────────────────────────────
# These produce the static libs consumed by all platform frontends:
#   Windows : ewpresenter.lib + libew.lib  → picked up by ewpresenter.net.vcxproj
#   macOS   : ewpresenter.a  + libew.a     → linked by Xcode
#   Linux   : ewpresenter.a  + libew.a     → linked by Qt CMake
add_subdirectory(libew)
add_subdirectory(ewpresenter)

# C bridge: platform-agnostic plain-C API over ewpresenter, consumed today by
# the macOS Swift frontend (built alongside native libs so the Xcode frontend
# project can import it with the correct deployment target, which the Xcode
# generator does not otherwise propagate) and available to any future
# frontend that needs a C ABI.
add_subdirectory(bridge)

# ── GUI frontends ─────────────────────────────────────────────────────────────
# Frontends are built by their native toolchains. CMake provides a convenience
# target on each platform so the full stack can be driven from one command:
#   cmake -B build && cmake --build build --target ewcalc_frontend

if(EWCALC_BUILD_FRONTEND)
    if(WIN32)
        # Locate MSBuild via the shared scripts/find-msbuild.ps1 helper (also
        # used by scripts/build-windows.ps1) so both share one lookup
        # implementation. It tries vswhere first, falling back to a
        # filesystem scan of the standard VS install roots — see that
        # script for the full rationale (vswhere's cached instance-state
        # reader can lag the actual installer version after an in-place VS
        # upgrade and silently report no installations even though VS is
        # fully usable).
        execute_process(
            COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File
                "${CMAKE_SOURCE_DIR}/scripts/find-msbuild.ps1"
            OUTPUT_VARIABLE MSBUILD
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )

        if(EXISTS "${MSBUILD}")
            set(WIN_SOLUTION
                "${CMAKE_SOURCE_DIR}/frontend/windows/ewcalc-winui/ewcalc-winui.sln")

            add_custom_target(ewcalc_frontend
                COMMAND "${MSBUILD}"
                    "${WIN_SOLUTION}"
                    /p:Configuration=${EWCALC_FRONTEND_CONFIG}
                    /p:Platform=x64
                    /m
                    /nologo
                    /verbosity:minimal
                DEPENDS ewpresenter          # ensures libs are built first
                COMMENT "Building Windows WinUI 3 frontend via MSBuild"
                VERBATIM
            )
            message(STATUS "ewcalc: Windows frontend target registered (MSBuild)")
        else()
            message(WARNING
                "ewcalc: MSBuild not found — ewcalc_frontend target unavailable.\n"
                "        Open frontend/windows/ewcalc-winui/ewcalc-winui.sln in Visual Studio.")
        endif()

    elseif(APPLE)
        # The SwiftUI frontend is a standalone CMake project (frontend/macos)
        # that needs the Xcode generator, so it cannot be add_subdirectory'd
        # into this build. Mirror the Windows pattern: a convenience target
        # that configures and builds it against this build tree's native libs,
        # with ad-hoc signing. Signed/notarized/packaged builds stay with
        # scripts/build-macos.sh, which owns identity detection and packaging.
        set(MACOS_FRONTEND_BUILD "${CMAKE_BINARY_DIR}/macos-frontend")
        # This build is normally single-config; forward its type to the Xcode
        # (multi-config) frontend build, defaulting to Release when unset.
        set(MACOS_FRONTEND_CONFIG "${CMAKE_BUILD_TYPE}")
        if(NOT MACOS_FRONTEND_CONFIG)
            set(MACOS_FRONTEND_CONFIG "Release")
        endif()

        add_custom_target(ewcalc_frontend
            COMMAND "${CMAKE_COMMAND}" -G Xcode
                -B "${MACOS_FRONTEND_BUILD}"
                -S "${CMAKE_SOURCE_DIR}/frontend/macos"
                -DEWCALC_NATIVE_BUILD_DIR=${CMAKE_BINARY_DIR}
            COMMAND "${CMAKE_COMMAND}" --build "${MACOS_FRONTEND_BUILD}"
                --config "${MACOS_FRONTEND_CONFIG}"
            DEPENDS ewpresenter ewcalc_bridge    # native libs + C bridge first
            COMMENT "Building macOS SwiftUI frontend via Xcode (ad-hoc signing)"
            VERBATIM
        )
        message(STATUS "ewcalc: macOS frontend target registered (Xcode, ad-hoc signing; use scripts/build-macos.sh for signed/packaged builds)")

    else()
        # The Qt6 frontend is likewise a standalone CMake project
        # (frontend/linux); scripts/build-linux.sh drives the two-stage build.
        message(STATUS "ewcalc: Linux frontend builds via scripts/build-linux.sh (standalone Qt6 project)")
    endif()
endif()
