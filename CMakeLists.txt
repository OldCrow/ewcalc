cmake_minimum_required(VERSION 3.20)
project(ewcalc VERSION 1.0.1 LANGUAGES CXX)

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
option(EWCALC_FRONTEND_CONFIG "MSBuild configuration (Windows)" "Release")
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
#   macOS   : ewpresenter.a  + libew.a     → linked by Xcode (Phase 4)
#   Linux   : ewpresenter.a  + libew.a     → linked by Qt CMake (Phase 5)
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
        # Locate MSBuild via vswhere
        set(VSWHERE "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer/vswhere.exe")
        if(EXISTS "${VSWHERE}")
            execute_process(
                COMMAND "${VSWHERE}" -latest
                    -requires Microsoft.Component.MSBuild
                    -property installationPath
                OUTPUT_VARIABLE VS_INSTALL_PATH
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            set(MSBUILD "${VS_INSTALL_PATH}/MSBuild/Current/Bin/MSBuild.exe")
        endif()

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
        # Phase 4 — Xcode project not yet created
        message(STATUS "ewcalc: macOS frontend (Phase 4) — not yet implemented")

    else()
        # Phase 5 — Qt6 CMake frontend not yet created
        message(STATUS "ewcalc: Linux frontend (Phase 5) — not yet implemented")
    endif()
endif()
