cmake_minimum_required(VERSION 3.20)
project(ewcalc-linux LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# ── Pre-built native library locations ────────────────────────────────────────
# libew and ewpresenter are built by the parent cmake invocation before this
# project is configured. On Linux, CMake prefixes the target name with 'lib':
#   target 'libew'       → liblibew.a
#   target 'ewpresenter' → libewpresenter.a
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
    IMPORTED_LOCATION
        "${EWCALC_NATIVE_BUILD_DIR}/lib/liblibew.a"
    INTERFACE_INCLUDE_DIRECTORIES
        "${EWCALC_SRC_ROOT}/libew/include"
)

add_library(ewpresenter_native STATIC IMPORTED)
set_target_properties(ewpresenter_native PROPERTIES
    IMPORTED_LOCATION
        "${EWCALC_NATIVE_BUILD_DIR}/lib/libewpresenter.a"
    INTERFACE_INCLUDE_DIRECTORIES
        "${EWCALC_SRC_ROOT}/ewpresenter/include"
)

# ── Sources ───────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/pages/PropagationPage.cpp
    src/pages/LinkPage.cpp
    src/pages/ReceiverPage.cpp
    src/pages/JammingPage.cpp
    src/pages/LocationPage.cpp
    src/pages/RadarPage.cpp
    src/pages/DigitalPage.cpp
    src/pages/ReferencePage.cpp
)

add_executable(ewcalc ${SOURCES})

target_include_directories(ewcalc PRIVATE src)

target_link_libraries(ewcalc PRIVATE
    Qt6::Widgets
    ewpresenter_native
    libew_native
)

# Qt headers use non-standard extensions; limit to -Wall/-Wextra/-Werror.
target_compile_options(ewcalc PRIVATE -Wall -Wextra -Werror)

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS ewcalc RUNTIME DESTINATION bin)
