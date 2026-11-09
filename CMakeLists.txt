cmake_minimum_required(VERSION 3.25)
project(Polymorphism LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_subdirectory(src)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest QUIET)
    if(GTest_FOUND)
        add_subdirectory(tests)
    else()
        message(STATUS "GTest not found; tests will not be built")
    endif()
endif()

function(polymorphism_sources_with_extension output_variable extension)
    set(collected "")
    foreach(target_name IN LISTS ARGN)
        if(NOT TARGET "${target_name}")
            continue()
        endif()

        get_target_property(target_sources "${target_name}" SOURCES)
        get_target_property(target_source_dir "${target_name}" SOURCE_DIR)
        foreach(source IN LISTS target_sources)
            if(source MATCHES "\\.${extension}$")
                if(NOT IS_ABSOLUTE "${source}")
                    cmake_path(ABSOLUTE_PATH source BASE_DIRECTORY "${target_source_dir}")
                endif()
                list(APPEND collected "${source}")
            endif()
        endforeach()
    endforeach()
    set("${output_variable}" "${collected}" PARENT_SCOPE)
endfunction()

set(POLYMORPHISM_TARGETS shapes draw_shapes)
if(TARGET shapes_tests)
    list(APPEND POLYMORPHISM_TARGETS shapes_tests)
endif()

polymorphism_sources_with_extension(POLYMORPHISM_CXX_FILES cpp ${POLYMORPHISM_TARGETS})
polymorphism_sources_with_extension(POLYMORPHISM_HEADER_FILES hpp ${POLYMORPHISM_TARGETS})

file(GLOB POLYMORPHISM_TEST_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/tests/*.cpp"
    "${CMAKE_SOURCE_DIR}/tests/*.hpp"
    "${CMAKE_SOURCE_DIR}/tests/*.h"
)
set(POLYMORPHISM_FORMAT_FILES
    ${POLYMORPHISM_CXX_FILES}
    ${POLYMORPHISM_HEADER_FILES}
    ${POLYMORPHISM_TEST_FILES}
)
list(REMOVE_DUPLICATES POLYMORPHISM_FORMAT_FILES)

find_program(CLANG_FORMAT_EXE NAMES clang-format clang-format-21)
find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-21)
find_program(CPPCHECK_EXE NAMES cppcheck)

if(CLANG_FORMAT_EXE)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i ${POLYMORPHISM_FORMAT_FILES}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Applying clang-format"
        COMMAND_EXPAND_LISTS
        USES_TERMINAL
        VERBATIM
    )
    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${POLYMORPHISM_FORMAT_FILES}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Checking clang-format"
        COMMAND_EXPAND_LISTS
        USES_TERMINAL
        VERBATIM
    )
endif()

if(CLANG_TIDY_EXE AND POLYMORPHISM_CXX_FILES)
    # -w keeps the compiler quiet inside the standard library. The build
    # already treats -Wall -Wextra -Wpedantic as errors.
    add_custom_target(tidy
        COMMAND ${CLANG_TIDY_EXE}
            --quiet
            --extra-arg=-w
            -p "${CMAKE_BINARY_DIR}"
            ${POLYMORPHISM_CXX_FILES}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Running clang-tidy"
        COMMAND_EXPAND_LISTS
        USES_TERMINAL
        VERBATIM
    )
endif()

# cppcheck 2.19 accepts --std=c++20 and does not know C++26 yet.
# The project is standard C++, so that profile still checks this code.
if(CPPCHECK_EXE)
    add_custom_target(cppcheck
        COMMAND ${CPPCHECK_EXE}
            --project=${CMAKE_BINARY_DIR}/compile_commands.json
            --quiet
            --enable=warning,style,performance,portability
            --std=c++20
            --check-level=exhaustive
            --error-exitcode=1
            --inline-suppr
            --suppress=missingIncludeSystem
            --suppress=unmatchedSuppression
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Running cppcheck"
        USES_TERMINAL
        VERBATIM
    )
endif()
