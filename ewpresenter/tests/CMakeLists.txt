add_executable(test_presenters test_presenters.cpp)
target_link_libraries(test_presenters PRIVATE ewpresenter)
target_include_directories(test_presenters PRIVATE
    ${PROJECT_SOURCE_DIR}/libew/tests   # for test_main.h
)
set_target_properties(test_presenters PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)
if(MSVC)
    target_compile_options(test_presenters PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_presenters PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME test_presenters COMMAND test_presenters)

# Formatter buffer-size tests
add_executable(test_formatter test_formatter.cpp)
target_link_libraries(test_formatter PRIVATE ewpresenter)
target_include_directories(test_formatter PRIVATE
    ${PROJECT_SOURCE_DIR}/libew/tests
)
set_target_properties(test_formatter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)
if(MSVC)
    target_compile_options(test_formatter PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_formatter PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME test_formatter COMMAND test_formatter)

# C bridge integration tests (macOS only — bridge is not built on other platforms)
if(APPLE)
    add_executable(test_bridge test_bridge.cpp)
    target_link_libraries(test_bridge PRIVATE ewcalc_bridge ewpresenter)
    target_include_directories(test_bridge PRIVATE
        ${PROJECT_SOURCE_DIR}/libew/tests
        ${PROJECT_SOURCE_DIR}/frontend/macos/bridge
    )
    set_target_properties(test_bridge PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
    )
    target_compile_options(test_bridge PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME test_bridge COMMAND test_bridge)
endif()
