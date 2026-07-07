get_filename_component(_TEST_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
add_executable(${_TEST_NAME}_tests
    ${_TEST_NAME}_tests_main.cpp
    test_download_path_utils.cpp
    test_update_version_parsing.cpp
    test_bambuddy_client.cpp
    test_orca_cloud_keychain.cpp
    )

if (MSVC)
    target_link_libraries(${_TEST_NAME}_tests Setupapi.lib)
endif ()

target_link_libraries(${_TEST_NAME}_tests test_common libslic3r_gui libslic3r Catch2::Catch2WithMain)
get_target_property(_libslic3r_gui_include_dirs libslic3r_gui INCLUDE_DIRECTORIES)
if (_libslic3r_gui_include_dirs)
    target_include_directories(${_TEST_NAME}_tests PRIVATE ${_libslic3r_gui_include_dirs})
endif ()
set_property(TARGET ${_TEST_NAME}_tests PROPERTY FOLDER "tests")

orcaslicer_copy_test_dlls()

orcaslicer_discover_tests(${_TEST_NAME}_tests)
