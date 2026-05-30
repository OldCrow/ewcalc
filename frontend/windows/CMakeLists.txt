include("${CMAKE_SOURCE_DIR}/cmake/platform_windows.cmake")

add_executable(ewcalc_windows WIN32
    main.cpp
    app.manifest   # embedded Win32 manifest (DPI awareness, compatibility)
)

set_target_properties(ewcalc_windows PROPERTIES
    OUTPUT_NAME "ewcalc"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(ewcalc_windows PRIVATE ewpresenter)

ewcalc_configure_winui3_target(ewcalc_windows)
