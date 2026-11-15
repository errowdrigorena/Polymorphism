cmake_minimum_required(VERSION 3.28)
project(Polymorphism LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

option(POLYMORPHISM_SANITIZERS "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(POLYMORPHISM_COVERAGE "Instrument the build with gcov" OFF)

if(POLYMORPHISM_COVERAGE AND POLYMORPHISM_SANITIZERS)
    message(FATAL_ERROR
        "POLYMORPHISM_COVERAGE and POLYMORPHISM_SANITIZERS need separate build directories.")
endif()

add_compile_options(-Wall -Wextra -Wpedantic -Werror)
if(POLYMORPHISM_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()
if(POLYMORPHISM_COVERAGE)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(FATAL_ERROR "POLYMORPHISM_COVERAGE uses gcov and requires GCC.")
    endif()
    # -fprofile-abs-path keeps gcovr pointed at this source tree.
    # Atomic updates survive ctest running several tests at once.
    add_compile_options(--coverage -fprofile-abs-path -fprofile-update=atomic)
    add_link_options(--coverage)
endif()

add_subdirectory(src)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest QUIET)
    if(GTest_FOUND)
        add_subdirectory(tests)
    elseif(POLYMORPHISM_COVERAGE)
        message(FATAL_ERROR
            "POLYMORPHISM_COVERAGE requires GTest so the tests can generate data.")
    else()
        message(STATUS "GTest not found; tests will not be built")
    endif()
elseif(POLYMORPHISM_COVERAGE)
    message(FATAL_ERROR "POLYMORPHISM_COVERAGE requires BUILD_TESTING.")
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

set(POLYMORPHISM_TARGETS shapes type_erasure draw_shapes draw_shapes_reflection)
if(TARGET shapes_tests)
    list(APPEND POLYMORPHISM_TARGETS shapes_tests)
endif()
if(TARGET type_erasure_shapes_tests)
    list(APPEND POLYMORPHISM_TARGETS type_erasure_shapes_tests)
endif()
if(TARGET reflection_shapes_tests)
    list(APPEND POLYMORPHISM_TARGETS reflection_shapes_tests)
endif()

polymorphism_sources_with_extension(POLYMORPHISM_CXX_FILES cpp ${POLYMORPHISM_TARGETS})
polymorphism_sources_with_extension(POLYMORPHISM_HEADER_FILES hpp ${POLYMORPHISM_TARGETS})
polymorphism_sources_with_extension(POLYMORPHISM_MODULE_FILES cppm ${POLYMORPHISM_TARGETS})

file(GLOB POLYMORPHISM_TEST_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/tests/*.cpp"
    "${CMAKE_SOURCE_DIR}/tests/*.hpp"
    "${CMAKE_SOURCE_DIR}/tests/*.h"
)
set(POLYMORPHISM_FORMAT_FILES
    ${POLYMORPHISM_CXX_FILES}
    ${POLYMORPHISM_HEADER_FILES}
    ${POLYMORPHISM_MODULE_FILES}
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

if(POLYMORPHISM_COVERAGE)
    get_filename_component(cxx_compiler_name "${CMAKE_CXX_COMPILER}" NAME)
    get_filename_component(cxx_compiler_directory "${CMAKE_CXX_COMPILER}" DIRECTORY)
    string(REGEX REPLACE "g\\+\\+(-[0-9]+)?$" "gcov\\1" gcov_name "${cxx_compiler_name}")
    find_program(
        GCOV_EXE
        NAMES "${gcov_name}" gcov
        HINTS "${cxx_compiler_directory}"
        REQUIRED
    )
    find_program(GCOVR_EXE NAMES gcovr REQUIRED)
    find_program(PYTHON3_EXE NAMES python3 REQUIRED)

    # gcov misattributes inline functions and templates in module interface
    # units, so this report stays informational and does not gate on a percentage.
    # main.cpp is the demo driver; the tests never execute it.
    # The SVG is the percentage shown in the README. CI commits it when the
    # rounded line percentage changes.
    add_custom_target(coverage
        COMMAND find "${CMAKE_BINARY_DIR}" -type f -name "*.gcda" -delete
        COMMAND "${CMAKE_CTEST_COMMAND}"
            --test-dir "${CMAKE_BINARY_DIR}"
            --output-on-failure
            --no-tests=error
        COMMAND "${GCOVR_EXE}"
            --gcov-executable "${GCOV_EXE}"
            --root "${CMAKE_SOURCE_DIR}"
            --filter "${CMAKE_SOURCE_DIR}/src"
            --exclude "${CMAKE_SOURCE_DIR}/src/main\\.cpp$"
            --exclude "${CMAKE_SOURCE_DIR}/src/Reflection_TypeErasure_Polymorphism/main\\.cpp$"
            --json-summary "${CMAKE_BINARY_DIR}/coverage-summary.json"
            --txt "${CMAKE_BINARY_DIR}/coverage.txt"
            "${CMAKE_BINARY_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E cat "${CMAKE_BINARY_DIR}/coverage.txt"
        COMMAND "${PYTHON3_EXE}"
            "${CMAKE_SOURCE_DIR}/tools/coverage_badge.py"
            "${CMAKE_BINARY_DIR}/coverage-summary.json"
            "${CMAKE_SOURCE_DIR}/badges/coverage.svg"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        DEPENDS
            shapes_tests
            type_erasure_shapes_tests
        COMMENT "Running tests and reporting coverage for src"
        USES_TERMINAL
        VERBATIM
    )
endif()
