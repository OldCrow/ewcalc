set(EWPRESENTER_SOURCES
    src/formatter.cpp
    src/propagation_presenter.cpp
    src/link_presenter.cpp
    src/receiver_presenter.cpp
    src/jamming_presenter.cpp
    src/location_presenter.cpp
    src/radar_presenter.cpp
    src/digital_presenter.cpp
    src/antenna_presenter.cpp
)

add_library(ewpresenter STATIC ${EWPRESENTER_SOURCES})

target_include_directories(ewpresenter PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(ewpresenter PUBLIC libew)
target_compile_features(ewpresenter PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(ewpresenter PRIVATE /W4 /WX /permissive-)
else()
    target_compile_options(ewpresenter PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_subdirectory(harness)

if(EWCALC_BUILD_TESTS)
    add_subdirectory(tests)
endif()
