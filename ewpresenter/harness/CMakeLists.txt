add_executable(ewpresenter_harness main.cpp)

if(EWCALC_BUILD_TESTS)
    # Smoke test: run the harness under ctest to catch crashes and segfaults.
    # Correctness assertions live in ewpresenter/tests/test_presenters.cpp.
    add_test(NAME ewpresenter_harness COMMAND ewpresenter_harness)
endif()
target_link_libraries(ewpresenter_harness PRIVATE ewpresenter)
set_target_properties(ewpresenter_harness PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
if(MSVC)
    target_compile_options(ewpresenter_harness PRIVATE /W4 /permissive-)
else()
    target_compile_options(ewpresenter_harness PRIVATE -Wall -Wextra -Wpedantic)
endif()
