set(BRIDGE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ewcalc_bridge.cpp)

add_library(ewcalc_bridge STATIC ${BRIDGE_SOURCES})

target_include_directories(ewcalc_bridge PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(ewcalc_bridge PRIVATE cxx_std_20)

target_link_libraries(ewcalc_bridge PRIVATE ewpresenter libew)

# Emit to the same lib/ output as ewpresenter and libew so the Xcode
# project can find it at EWCALC_NATIVE_BUILD_DIR/lib/libewcalc_bridge.a
set_target_properties(ewcalc_bridge PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

if(MSVC)
    target_compile_options(ewcalc_bridge PRIVATE /W4)
else()
    target_compile_options(ewcalc_bridge PRIVATE -Wall -Wextra -Werror)
endif()
