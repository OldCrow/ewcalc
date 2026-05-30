add_executable(ewpresenter_harness main.cpp)
target_link_libraries(ewpresenter_harness PRIVATE ewpresenter)
set_target_properties(ewpresenter_harness PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
if(MSVC)
    target_compile_options(ewpresenter_harness PRIVATE /W4 /permissive-)
else()
    target_compile_options(ewpresenter_harness PRIVATE -Wall -Wextra -Wpedantic)
endif()
