set(LIBEW_SOURCES
    src/propagation.cpp
    src/antenna.cpp
    src/link.cpp
    src/receiver.cpp
    src/jamming.cpp
    src/location.cpp
    src/radar.cpp
)

add_library(libew STATIC ${LIBEW_SOURCES})

target_include_directories(libew PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(libew PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(libew PRIVATE /W4 /WX /permissive-)
else()
    target_compile_options(libew PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(EWCALC_BUILD_TESTS)
    add_subdirectory(tests)
endif()
