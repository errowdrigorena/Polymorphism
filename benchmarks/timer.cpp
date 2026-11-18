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
    measure<sweep_draw_direct>(state, draw_bytes().direct);
}

void draw_traditional_polymorphism(benchmark::State& state)
{
    measure<sweep_draw_traditional_polymorphism>(state, draw_bytes().traditional_polymorphism);
}

void draw_type_erasure(benchmark::State& state)
{
    measure<sweep_draw_type_erasure>(state, draw_bytes().type_erasure);
}

void draw_std_polymorphic(benchmark::State& state)
{
    measure<sweep_draw_std_polymorphic>(state, draw_bytes().std_polymorphic);
}

void draw_reflection_type_erasure(benchmark::State& state)
{
    measure<sweep_draw_reflection_type_erasure>(state, draw_bytes().reflection_type_erasure);
}

void area_direct(benchmark::State& state)
{
    measure<sweep_area_direct>(state, area_bytes().direct);
}

void area_traditional_polymorphism(benchmark::State& state)
{
    measure<sweep_area_traditional_polymorphism>(state, area_bytes().traditional_polymorphism);
}

void area_type_erasure(benchmark::State& state)
{
    measure<sweep_area_type_erasure>(state, area_bytes().type_erasure);
}

void area_std_polymorphic(benchmark::State& state)
{
    measure<sweep_area_std_polymorphic>(state, area_bytes().std_polymorphic);
}

void area_reflection_type_erasure(benchmark::State& state)
{
    measure<sweep_area_reflection_type_erasure>(state, area_bytes().reflection_type_erasure);
}

void copy_direct_bench(benchmark::State& state)
{
    measure<copy_direct>(state, copy_bytes().direct);
}

void copy_traditional_polymorphism_bench(benchmark::State& state)
{
    measure<copy_traditional_polymorphism>(state, copy_bytes().traditional_polymorphism);
}

void copy_type_erasure_bench(benchmark::State& state)
{
    measure<copy_type_erasure>(state, copy_bytes().type_erasure);
}

void copy_std_polymorphic_bench(benchmark::State& state)
{
    measure<copy_std_polymorphic>(state, copy_bytes().std_polymorphic);
}

void copy_reflection_type_erasure_bench(benchmark::State& state)
{
    measure<copy_reflection_type_erasure>(state, copy_bytes().reflection_type_erasure);
}

void build_direct_bench(benchmark::State& state)
{
    measure<build_direct>(state, build_bytes().direct);
}

void build_traditional_polymorphism_bench(benchmark::State& state)
{
    measure<build_traditional_polymorphism>(state, build_bytes().traditional_polymorphism);
}

void build_type_erasure_bench(benchmark::State& state)
{
    measure<build_type_erasure>(state, build_bytes().type_erasure);
}

void build_std_polymorphic_bench(benchmark::State& state)
{
    measure<build_std_polymorphic>(state, build_bytes().std_polymorphic);
}

void build_reflection_type_erasure_bench(benchmark::State& state)
{
    measure<build_reflection_type_erasure>(state, build_bytes().reflection_type_erasure);
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

template <unsigned long long (*run)()>
void measure_cycle(benchmark::State& state, unsigned long bytes)
{
    measure<run>(state, bytes, true);
}

void cycle_direct_5_bench(benchmark::State& state)
{
    measure_cycle<cycle_direct_5>(state, cycle_bytes().direct);
}
void cycle_traditional_polymorphism_5_bench(benchmark::State& state)
{
    measure_cycle<cycle_traditional_polymorphism_5>(state, cycle_bytes().traditional_polymorphism);
}
void cycle_type_erasure_5_bench(benchmark::State& state)
{
    measure_cycle<cycle_type_erasure_5>(state, cycle_bytes().type_erasure);
}
void cycle_std_polymorphic_5_bench(benchmark::State& state)
{
    measure_cycle<cycle_std_polymorphic_5>(state, cycle_bytes().std_polymorphic);
}
void cycle_reflection_type_erasure_5_bench(benchmark::State& state)
{
    measure_cycle<cycle_reflection_type_erasure_5>(state, cycle_bytes().reflection_type_erasure);
}

void cycle_direct_50_bench(benchmark::State& state)
{
    measure_cycle<cycle_direct_50>(state, cycle_bytes().direct);
}
void cycle_traditional_polymorphism_50_bench(benchmark::State& state)
{
    measure_cycle<cycle_traditional_polymorphism_50>(state, cycle_bytes().traditional_polymorphism);
}
void cycle_type_erasure_50_bench(benchmark::State& state)
{
    measure_cycle<cycle_type_erasure_50>(state, cycle_bytes().type_erasure);
}
void cycle_std_polymorphic_50_bench(benchmark::State& state)
{
    measure_cycle<cycle_std_polymorphic_50>(state, cycle_bytes().std_polymorphic);
}
void cycle_reflection_type_erasure_50_bench(benchmark::State& state)
{
    measure_cycle<cycle_reflection_type_erasure_50>(state, cycle_bytes().reflection_type_erasure);
}

void cycle_direct_500_bench(benchmark::State& state)
{
    measure_cycle<cycle_direct_500>(state, cycle_bytes().direct);
}
void cycle_traditional_polymorphism_500_bench(benchmark::State& state)
{
    measure_cycle<cycle_traditional_polymorphism_500>(state,
                                                      cycle_bytes().traditional_polymorphism);
}
void cycle_type_erasure_500_bench(benchmark::State& state)
{
    measure_cycle<cycle_type_erasure_500>(state, cycle_bytes().type_erasure);
}
void cycle_std_polymorphic_500_bench(benchmark::State& state)
{
    measure_cycle<cycle_std_polymorphic_500>(state, cycle_bytes().std_polymorphic);
}
void cycle_reflection_type_erasure_500_bench(benchmark::State& state)
{
    measure_cycle<cycle_reflection_type_erasure_500>(state, cycle_bytes().reflection_type_erasure);
}

void cycle_direct_5000_bench(benchmark::State& state)
{
    measure_cycle<cycle_direct_5000>(state, cycle_bytes().direct);
}
void cycle_traditional_polymorphism_5000_bench(benchmark::State& state)
{
    measure_cycle<cycle_traditional_polymorphism_5000>(state,
                                                       cycle_bytes().traditional_polymorphism);
}
void cycle_type_erasure_5000_bench(benchmark::State& state)
{
    measure_cycle<cycle_type_erasure_5000>(state, cycle_bytes().type_erasure);
}
void cycle_std_polymorphic_5000_bench(benchmark::State& state)
{
    measure_cycle<cycle_std_polymorphic_5000>(state, cycle_bytes().std_polymorphic);
}
void cycle_reflection_type_erasure_5000_bench(benchmark::State& state)
{
    measure_cycle<cycle_reflection_type_erasure_5000>(state, cycle_bytes().reflection_type_erasure);
}

BENCHMARK(draw_direct)->Name("draw/direct");
BENCHMARK(draw_traditional_polymorphism)->Name("draw/traditional_polymorphism");
BENCHMARK(draw_type_erasure)->Name("draw/type_erasure");
BENCHMARK(draw_std_polymorphic)->Name("draw/std::polymorphic");
BENCHMARK(draw_reflection_type_erasure)->Name("draw/reflection_type_erasure");
BENCHMARK(area_direct)->Name("area/direct");
BENCHMARK(area_traditional_polymorphism)->Name("area/traditional_polymorphism");
BENCHMARK(area_type_erasure)->Name("area/type_erasure");
BENCHMARK(area_std_polymorphic)->Name("area/std::polymorphic");
BENCHMARK(area_reflection_type_erasure)->Name("area/reflection_type_erasure");
BENCHMARK(copy_direct_bench)->Name("copy/direct");
BENCHMARK(copy_traditional_polymorphism_bench)->Name("copy/traditional_polymorphism");
BENCHMARK(copy_type_erasure_bench)->Name("copy/type_erasure");
BENCHMARK(copy_std_polymorphic_bench)->Name("copy/std::polymorphic");
BENCHMARK(copy_reflection_type_erasure_bench)->Name("copy/reflection_type_erasure");
BENCHMARK(build_direct_bench)->Name("build/direct");
BENCHMARK(build_traditional_polymorphism_bench)->Name("build/traditional_polymorphism");
BENCHMARK(build_type_erasure_bench)->Name("build/type_erasure");
BENCHMARK(build_std_polymorphic_bench)->Name("build/std::polymorphic");
BENCHMARK(build_reflection_type_erasure_bench)->Name("build/reflection_type_erasure");
BENCHMARK(cycle_direct_bench)->Name("cycle/direct");
BENCHMARK(cycle_traditional_polymorphism_bench)->Name("cycle/traditional_polymorphism");
BENCHMARK(cycle_type_erasure_bench)->Name("cycle/type_erasure");
BENCHMARK(cycle_std_polymorphic_bench)->Name("cycle/std::polymorphic");
BENCHMARK(cycle_reflection_type_erasure_bench)->Name("cycle/reflection_type_erasure");
BENCHMARK(cycle_direct_5_bench)->Name("cycle_5/direct");
BENCHMARK(cycle_traditional_polymorphism_5_bench)->Name("cycle_5/traditional_polymorphism");
BENCHMARK(cycle_type_erasure_5_bench)->Name("cycle_5/type_erasure");
BENCHMARK(cycle_std_polymorphic_5_bench)->Name("cycle_5/std::polymorphic");
BENCHMARK(cycle_reflection_type_erasure_5_bench)->Name("cycle_5/reflection_type_erasure");
BENCHMARK(cycle_direct_50_bench)->Name("cycle_50/direct");
BENCHMARK(cycle_traditional_polymorphism_50_bench)->Name("cycle_50/traditional_polymorphism");
BENCHMARK(cycle_type_erasure_50_bench)->Name("cycle_50/type_erasure");
BENCHMARK(cycle_std_polymorphic_50_bench)->Name("cycle_50/std::polymorphic");
BENCHMARK(cycle_reflection_type_erasure_50_bench)->Name("cycle_50/reflection_type_erasure");
BENCHMARK(cycle_direct_500_bench)->Name("cycle_500/direct");
BENCHMARK(cycle_traditional_polymorphism_500_bench)->Name("cycle_500/traditional_polymorphism");
BENCHMARK(cycle_type_erasure_500_bench)->Name("cycle_500/type_erasure");
BENCHMARK(cycle_std_polymorphic_500_bench)->Name("cycle_500/std::polymorphic");
BENCHMARK(cycle_reflection_type_erasure_500_bench)->Name("cycle_500/reflection_type_erasure");
BENCHMARK(cycle_direct_5000_bench)->Name("cycle_5000/direct");
BENCHMARK(cycle_traditional_polymorphism_5000_bench)->Name("cycle_5000/traditional_polymorphism");
BENCHMARK(cycle_type_erasure_5000_bench)->Name("cycle_5000/type_erasure");
BENCHMARK(cycle_std_polymorphic_5000_bench)->Name("cycle_5000/std::polymorphic");
BENCHMARK(cycle_reflection_type_erasure_5000_bench)->Name("cycle_5000/reflection_type_erasure");

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
        "draw", {sweep_draw_direct, sweep_draw_traditional_polymorphism, sweep_draw_type_erasure,
                 sweep_draw_std_polymorphic, sweep_draw_reflection_type_erasure});
    const bool area_ok = group_matches(
        "area", {sweep_area_direct, sweep_area_traditional_polymorphism, sweep_area_type_erasure,
                 sweep_area_std_polymorphic, sweep_area_reflection_type_erasure});
    const bool copy_ok = group_matches(
        "copy", {copy_direct, copy_traditional_polymorphism, copy_type_erasure,
                 copy_std_polymorphic, copy_reflection_type_erasure});
    const bool build_ok =
        group_matches("build", {build_direct, build_traditional_polymorphism, build_type_erasure,
                                 build_std_polymorphic, build_reflection_type_erasure});
    const bool cycle_ok = group_matches(
        "cycle", {cycle_direct, cycle_traditional_polymorphism, cycle_type_erasure,
                  cycle_std_polymorphic, cycle_reflection_type_erasure});
    const bool cycle_5_ok = group_matches(
        "cycle_5", {cycle_direct_5, cycle_traditional_polymorphism_5, cycle_type_erasure_5,
                    cycle_std_polymorphic_5, cycle_reflection_type_erasure_5});
    const bool cycle_50_ok = group_matches(
        "cycle_50", {cycle_direct_50, cycle_traditional_polymorphism_50, cycle_type_erasure_50,
                     cycle_std_polymorphic_50, cycle_reflection_type_erasure_50});
    const bool cycle_500_ok = group_matches(
        "cycle_500",
        {cycle_direct_500, cycle_traditional_polymorphism_500, cycle_type_erasure_500,
         cycle_std_polymorphic_500, cycle_reflection_type_erasure_500});
    const bool cycle_5000_ok = group_matches(
        "cycle_5000",
        {cycle_direct_5000, cycle_traditional_polymorphism_5000, cycle_type_erasure_5000,
         cycle_std_polymorphic_5000, cycle_reflection_type_erasure_5000});
    if (!draw_ok || !area_ok || !copy_ok || !build_ok || !cycle_ok || !cycle_5_ok || !cycle_50_ok ||
        !cycle_500_ok || !cycle_5000_ok) {
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
