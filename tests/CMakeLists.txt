find_package(GTest REQUIRED)

add_executable(shapes_tests
    test_shapes.cpp
)

if(DEFINED gtest_PACKAGE_FOLDER_RELEASE)
    set(GTEST_CONAN_INCLUDE "${gtest_PACKAGE_FOLDER_RELEASE}/include")
    set(GTEST_CONAN_LIB_DIR "${gtest_PACKAGE_FOLDER_RELEASE}/lib")

    target_include_directories(shapes_tests PRIVATE "${GTEST_CONAN_INCLUDE}")
    target_link_libraries(shapes_tests
        PRIVATE
            shapes
            "${GTEST_CONAN_LIB_DIR}/libgtest_main.a"
            "${GTEST_CONAN_LIB_DIR}/libgtest.a"
            pthread
            m
    )
else()
    target_link_libraries(shapes_tests
        PRIVATE
            shapes
            GTest::Main
    )
endif()

include(GoogleTest)
gtest_discover_tests(shapes_tests)
