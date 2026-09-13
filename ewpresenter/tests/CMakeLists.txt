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

# Reference data layer tests (#73). UTF-8 literals → /utf-8 on MSVC.
add_executable(test_reference test_reference.cpp)
target_link_libraries(test_reference PRIVATE ewpresenter)
target_include_directories(test_reference PRIVATE
    ${PROJECT_SOURCE_DIR}/libew/tests
)
set_target_properties(test_reference PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)
if(MSVC)
    target_compile_options(test_reference PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(test_reference PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME test_reference COMMAND test_reference)

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

# C bridge integration tests. The bridge is plain C++20 with no platform-
# specific code, so it's built and tested on every platform.
add_executable(test_bridge test_bridge.cpp)
target_link_libraries(test_bridge PRIVATE ewcalc_bridge ewpresenter)
target_include_directories(test_bridge PRIVATE
    ${PROJECT_SOURCE_DIR}/libew/tests
    ${PROJECT_SOURCE_DIR}/bridge
)
set_target_properties(test_bridge PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)
if(MSVC)
    target_compile_options(test_bridge PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_bridge PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME test_bridge COMMAND test_bridge)
