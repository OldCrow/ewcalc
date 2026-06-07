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
