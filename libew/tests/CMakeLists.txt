set(LIBEW_TEST_SOURCES
    test_units.cpp
    test_propagation.cpp
    test_receiver.cpp
    test_jamming.cpp
    test_location.cpp
    test_radar.cpp
)

foreach(test_src ${LIBEW_TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} PRIVATE libew)
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(${test_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
    )
    if(MSVC)
        target_compile_options(${test_name} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${test_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
