find_package(GTest REQUIRED)

include(GoogleTest)

function(polymorphism_add_gtest target source)
    add_executable(${target} ${source})

    if(TARGET GTest::gtest_main)
        set(gtest_main GTest::gtest_main)
    else()
        set(gtest_main GTest::Main)
    endif()

    target_link_libraries(${target}
        PRIVATE
            ${ARGN}
            GTest::gmock
            ${gtest_main}
    )

    gtest_discover_tests(${target})
endfunction()

polymorphism_add_gtest(shapes_tests test_shapes.cpp shapes)
target_sources(shapes_tests PRIVATE shapes_cases.cpp)
polymorphism_add_gtest(type_erasure_shapes_tests test_type_erasure_shapes.cpp shapes type_erasure)
target_sources(type_erasure_shapes_tests PRIVATE type_erasure_shapes_cases.cpp)
polymorphism_add_gtest(polymorphic_shapes_tests test_polymorphic_shapes.cpp polymorphic)
target_sources(polymorphic_shapes_tests PRIVATE polymorphic_shapes_cases.cpp)
polymorphism_add_gtest(reflection_shapes_tests test_reflection_shapes.cpp reflection_type_erasure)
target_sources(reflection_shapes_tests PRIVATE reflection_shapes_cases.cpp)
