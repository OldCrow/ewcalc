# EwcalcVersion.cmake
# Sets EWCALC_VERSION from the root CMakeLists.txt project(ewcalc VERSION ...).
#
# The macOS and Linux frontends are configured as standalone CMake projects,
# so they cannot inherit the root project's version. Each used to carry its
# own copy, and the copies drifted: v1.3.0 shipped a DMG with no bundle
# version and an AppImage binary stamped 1.1.0. Including this module keeps
# the root CMakeLists.txt the single source for every CMake-built artifact.
#
# Usage (before project()):
#   include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/EwcalcVersion.cmake")

file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/../CMakeLists.txt" _ewcalc_project_line
     REGEX "^project\\(ewcalc VERSION [0-9]+\\.[0-9]+\\.[0-9]+")
string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" EWCALC_VERSION "${_ewcalc_project_line}")
if(NOT EWCALC_VERSION)
    message(FATAL_ERROR
        "EwcalcVersion: no 'project(ewcalc VERSION X.Y.Z' line in the root CMakeLists.txt")
endif()
unset(_ewcalc_project_line)
