find_package(benchmark QUIET)
if(NOT benchmark_FOUND)
    message(STATUS "benchmark not found; polymorphism_benchmarks will not be built")
    return()
endif()

# Google Benchmark's headers and `import std;` cannot share a translation unit
# with this GCC, the same limit as GoogleTest. The benchmark_work interface
# therefore does not import std. timer.cpp includes the library and imports
# that interface. benchmark_work.cpp is the implementation unit: it imports std
# and the shape modules. Leave interprocedural optimization off so the timed
# bodies stay out of the translation unit that runs the benchmark loop.
add_executable(polymorphism_benchmarks timer.cpp benchmark_work.cpp)
target_sources(polymorphism_benchmarks
    PRIVATE
        FILE_SET CXX_MODULES FILES
            benchmark_work.cppm
)
target_link_libraries(polymorphism_benchmarks
    PRIVATE
        benchmark::benchmark
        shapes
        type_erasure
        polymorphic
        reflection_type_erasure
)
set_target_properties(polymorphism_benchmarks PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    INTERPROCEDURAL_OPTIMIZATION OFF
)
