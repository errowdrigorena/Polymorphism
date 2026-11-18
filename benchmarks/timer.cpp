#include <benchmark/benchmark.h>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <iostream>

import benchmark_work;

namespace {

// One Google Benchmark iteration is one pass over object_count values.
// items_per_second is then one draw(), area(), copy, or build.
// A cycle pass reports ns_per_cycle instead: the time for one construct,
// draw(), area(), and destroy.
template <unsigned long long (*run)()>
void measure(benchmark::State& state, unsigned long bytes, bool per_cycle = false)
{
    for ([[maybe_unused]] auto _ : state) {
        auto sink = run();
        benchmark::DoNotOptimize(sink);
    }
    if (per_cycle) {
        state.counters["ns_per_cycle"] = benchmark::Counter(
            static_cast<double>(object_count) / 1e9,
            benchmark::Counter::kIsIterationInvariantRate | benchmark::Counter::kInvert);
    }
    else {
        state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) *
                                static_cast<int64_t>(object_count));
    }
    state.counters["bytes"] =
        benchmark::Counter(static_cast<double>(bytes), benchmark::Counter::kAvgThreads);
}

bool group_matches(const char* name, std::initializer_list<unsigned long long (*)()> runs)
{
    unsigned long long expected = 0;
    bool have_expected = false;
    for (auto* run : runs) {
        const auto first = run();
        const auto second = run();
        if (first != second) {
            std::cerr << name << " checksum changed between passes\n";
            return false;
        }
        if (!have_expected) {
            expected = first;
            have_expected = true;
        }
        else if (first != expected) {
            std::cerr << name << " checksums differ\n";
            return false;
        }
    }
    return true;
}

void draw_direct(benchmark::State& state)
{
    measure<sweep_direct_draw>(state, benchmark_sizes().value);
}

void draw_shapes(benchmark::State& state)
{
    measure<sweep_shapes>(state, benchmark_sizes().shape);
}

void draw_type_erasure(benchmark::State& state)
{
    measure<sweep_type_erasure>(state, benchmark_sizes().type_erasure);
}

void draw_polymorphic(benchmark::State& state)
{
    measure<sweep_polymorphic>(state, benchmark_sizes().polymorphic);
}

void area_direct(benchmark::State& state)
{
    measure<sweep_direct_area>(state, benchmark_sizes().scalar);
}

void area_virtual(benchmark::State& state)
{
    measure<sweep_virtual_area>(state, benchmark_sizes().area);
}

void area_reflection(benchmark::State& state)
{
    measure<sweep_reflection>(state, benchmark_sizes().reflection);
}

void copy_direct_bench(benchmark::State& state)
{
    measure<copy_direct>(state, benchmark_sizes().value);
}

void copy_type_erasure_bench(benchmark::State& state)
{
    measure<copy_type_erasure>(state, benchmark_sizes().type_erasure);
}

void copy_polymorphic_bench(benchmark::State& state)
{
    measure<copy_polymorphic>(state, benchmark_sizes().polymorphic);
}

void copy_reflection_bench(benchmark::State& state)
{
    measure<copy_reflection>(state, benchmark_sizes().reflection);
}

void build_direct_bench(benchmark::State& state)
{
    measure<build_direct>(state, benchmark_sizes().value);
}

void build_shapes_bench(benchmark::State& state)
{
    measure<build_shapes>(state, benchmark_sizes().shape);
}

void build_type_erasure_bench(benchmark::State& state)
{
    measure<build_type_erasure>(state, benchmark_sizes().type_erasure);
}

void build_polymorphic_bench(benchmark::State& state)
{
    measure<build_polymorphic>(state, benchmark_sizes().polymorphic);
}

void build_reflection_bench(benchmark::State& state)
{
    measure<build_reflection>(state, benchmark_sizes().reflection);
}

void cycle_direct_bench(benchmark::State& state)
{
    measure<cycle_direct>(state, cycle_bytes().direct, true);
}

void cycle_traditional_polymorphism_bench(benchmark::State& state)
{
    measure<cycle_traditional_polymorphism>(state, cycle_bytes().traditional_polymorphism, true);
}

void cycle_type_erasure_bench(benchmark::State& state)
{
    measure<cycle_type_erasure>(state, cycle_bytes().type_erasure, true);
}

void cycle_std_polymorphic_bench(benchmark::State& state)
{
    measure<cycle_std_polymorphic>(state, cycle_bytes().std_polymorphic, true);
}

void cycle_reflection_type_erasure_bench(benchmark::State& state)
{
    measure<cycle_reflection_type_erasure>(state, cycle_bytes().reflection_type_erasure, true);
}

BENCHMARK(draw_direct)->Name("draw/direct");
BENCHMARK(draw_shapes)->Name("draw/shapes");
BENCHMARK(draw_type_erasure)->Name("draw/type_erasure");
BENCHMARK(draw_polymorphic)->Name("draw/polymorphic");
BENCHMARK(area_direct)->Name("area/direct");
BENCHMARK(area_virtual)->Name("area/virtual");
BENCHMARK(area_reflection)->Name("area/reflection_type_erasure");
BENCHMARK(copy_direct_bench)->Name("copy/direct");
BENCHMARK(copy_type_erasure_bench)->Name("copy/type_erasure");
BENCHMARK(copy_polymorphic_bench)->Name("copy/polymorphic");
BENCHMARK(copy_reflection_bench)->Name("copy/reflection_type_erasure");
BENCHMARK(build_direct_bench)->Name("build/direct");
BENCHMARK(build_shapes_bench)->Name("build/shapes");
BENCHMARK(build_type_erasure_bench)->Name("build/type_erasure");
BENCHMARK(build_polymorphic_bench)->Name("build/polymorphic");
BENCHMARK(build_reflection_bench)->Name("build/reflection_type_erasure");
BENCHMARK(cycle_direct_bench)->Name("cycle/direct");
BENCHMARK(cycle_traditional_polymorphism_bench)->Name("cycle/traditional_polymorphism");
BENCHMARK(cycle_type_erasure_bench)->Name("cycle/type_erasure");
BENCHMARK(cycle_std_polymorphic_bench)->Name("cycle/std::polymorphic");
BENCHMARK(cycle_reflection_type_erasure_bench)->Name("cycle/reflection_type_erasure");

} // namespace

volatile unsigned long long kept = 0;

void keep_result(const char* data, unsigned long size)
{
    unsigned long long bits = size;
    if (size != 0) {
        bits += static_cast<unsigned char>(data[0]);
    }
    kept = bits;
}

int main(int argc, char** argv)
{
#ifndef NDEBUG
    std::cerr << "Debug build: timings are not useful. Use the gcc16-benchmarks preset.\n";
#endif

    prepare_benchmarks();
    const bool draw_ok = group_matches(
        "draw", {sweep_direct_draw, sweep_shapes, sweep_type_erasure, sweep_polymorphic});
    const bool area_ok =
        group_matches("area", {sweep_direct_area, sweep_virtual_area, sweep_reflection});
    const bool cycle_ok = group_matches(
        "cycle", {cycle_direct, cycle_traditional_polymorphism, cycle_type_erasure,
                  cycle_std_polymorphic, cycle_reflection_type_erasure});
    if (!draw_ok || !area_ok || !cycle_ok) {
        return EXIT_FAILURE;
    }

    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return EXIT_FAILURE;
    }
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return EXIT_SUCCESS;
}
