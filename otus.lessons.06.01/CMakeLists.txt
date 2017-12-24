add_executable(
    allocator
        "otus.lessons.06.01.cpp"
        "page.cpp"
)

add_executable(
    allocator_test
        "test.cpp"
        "page.cpp"
)


set_target_properties(allocator allocator_test PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS -Wpedantic -Wall -Wextra
)

set_target_properties(allocator allocator_test PROPERTIES
    COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
    INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)

target_link_libraries(allocator
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

target_link_libraries(allocator_test
    ${Boost_LIBRARIES}
)

add_test(allocator allocator_test)