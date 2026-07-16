cmake_minimum_required(VERSION 3.20)
# Configured as a standalone CMake project (see scripts/build-linux.sh), so it
# does not inherit the root CMakeLists.txt's project(ewcalc VERSION ...) —
# keep this version in sync with that one and with CHANGELOG.md.
project(ewcalc-linux VERSION 1.0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

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
    src/Settings.cpp
    src/pages/PropagationPage.cpp
    src/pages/LinkPage.cpp
    src/pages/ReceiverPage.cpp
    src/pages/JammingPage.cpp
    src/pages/LocationPage.cpp
    src/pages/RadarPage.cpp
    src/pages/DigitalPage.cpp
    src/pages/AntennaPage.cpp
    src/pages/ReferencePage.cpp
    resources/icons.qrc
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

# Desktop entry + hicolor icons, so a .deb/.rpm install registers the app with
# the system icon theme and application menu (Icon=ewcalc in ewcalc.desktop
# resolves via share/icons/hicolor/**/apps/ewcalc.png). The AppImage packaging
# path (scripts/build-linux.sh) handles this separately via linuxdeploy's
# --icon-file/--desktop-file flags and does not use this install() rule.
set(EWCALC_LINUX_ASSETS "${EWCALC_SRC_ROOT}/assets/linux")

install(FILES "${EWCALC_LINUX_ASSETS}/ewcalc.desktop"
    DESTINATION share/applications)

foreach(_size 16x16 32x32 48x48 64x64 128x128 256x256 512x512)
    install(FILES "${EWCALC_LINUX_ASSETS}/hicolor/${_size}/apps/ewcalc.png"
        DESTINATION "share/icons/hicolor/${_size}/apps")
endforeach()

# ── CPack: .deb / .rpm packaging ─────────────────────────────────────────────
# scripts/build-linux.sh invokes `cpack -G DEB` or `cpack -G RPM` directly
# (overriding the generator per invocation) so either format can be produced
# from the same configured build.
set(CPACK_PACKAGE_NAME "ewcalc")
set(CPACK_PACKAGE_VENDOR "OldCrow")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Electronic Warfare engineering calculator")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/OldCrow/ewcalc")
set(CPACK_RESOURCE_FILE_LICENSE "${EWCALC_SRC_ROOT}/LICENSE")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

# Debian/Ubuntu
# NOTE: CPACK_DEBIAN_PACKAGE_MAINTAINER's address is a placeholder — replace
# with a real contact before uploading to an actual Debian/Ubuntu archive.
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Gary Wolfman <noreply@example.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
# Qt6 runtime package names vary by distro release; verify on the target
# distro and adjust (e.g. Debian/Ubuntu commonly ship libqt6widgets6).
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6")
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")

# Fedora/openSUSE
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Engineering")
# As above, verify the Qt6 runtime package name on the target distro
# (e.g. Fedora ships qt6-qtbase-gui).
set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase-gui")
set(CPACK_RPM_FILE_NAME "RPM-DEFAULT")

include(CPack)
