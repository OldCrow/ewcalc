cmake_minimum_required(VERSION 3.20)
project(ewcalc VERSION 0.1.0 LANGUAGES CXX)

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

# Options
option(EWCALC_BUILD_TESTS "Build libew test suite" ON)
option(EWCALC_BUILD_GUI   "Build platform GUI frontend" OFF)

# Test support — must be at root for CTest discovery
if(EWCALC_BUILD_TESTS)
    enable_testing()
endif()

# Core library (always built)
add_subdirectory(libew)

# GUI frontend — platform-specific, Phase 3+
if(EWCALC_BUILD_GUI)
    if(WIN32)
        add_subdirectory(frontend/windows)
    elseif(APPLE)
        add_subdirectory(frontend/macos)
    else()
        add_subdirectory(frontend/linux)
    endif()
endif()
