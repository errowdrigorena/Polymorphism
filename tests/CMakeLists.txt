find_package(GTest REQUIRED)

include(GoogleTest)

function(polymorphism_add_gtest target source)
    add_executable(${target} ${source})

    if(DEFINED gtest_PACKAGE_FOLDER_RELEASE)
        target_include_directories(${target} PRIVATE "${gtest_PACKAGE_FOLDER_RELEASE}/include")
        target_link_libraries(${target}
            PRIVATE
                ${ARGN}
                "${gtest_PACKAGE_FOLDER_RELEASE}/lib/libgtest_main.a"
                "${gtest_PACKAGE_FOLDER_RELEASE}/lib/libgtest.a"
                pthread
                m
        )
    else()
        if(TARGET GTest::gtest_main)
            set(gtest_main GTest::gtest_main)
        else()
            set(gtest_main GTest::Main)
        endif()

        target_link_libraries(${target}
            PRIVATE
                ${ARGN}
                ${gtest_main}
        )
    endif()

    gtest_discover_tests(${target})
endfunction()

polymorphism_add_gtest(shapes_tests test_shapes.cpp shapes)
polymorphism_add_gtest(type_erasure_shapes_tests test_type_erasure_shapes.cpp shapes type_erasure)
