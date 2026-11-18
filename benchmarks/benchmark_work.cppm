export module benchmark_work;

// This interface stays free of import std. timer.cpp imports it beside the
// Google Benchmark headers, and this GCC rejects that mix.

export constexpr unsigned long object_count = 12288;

export struct BenchmarkSizes {
    unsigned long value;
    unsigned long shape;
    unsigned long type_erasure;
    unsigned long polymorphic;
    unsigned long scalar;
    unsigned long area;
    unsigned long reflection;
};

export struct CycleBytes {
    unsigned long direct;
    unsigned long traditional_polymorphism;
    unsigned long type_erasure;
    unsigned long std_polymorphic;
    unsigned long reflection_type_erasure;
};

export void prepare_benchmarks();
export BenchmarkSizes benchmark_sizes();
export CycleBytes cycle_bytes();

export unsigned long long sweep_direct_draw();
export unsigned long long sweep_shapes();
export unsigned long long sweep_type_erasure();
export unsigned long long sweep_polymorphic();
export unsigned long long sweep_direct_area();
export unsigned long long sweep_virtual_area();
export unsigned long long sweep_reflection();
export unsigned long long copy_direct();
export unsigned long long copy_type_erasure();
export unsigned long long copy_polymorphic();
export unsigned long long copy_reflection();
export unsigned long long build_direct();
export unsigned long long build_shapes();
export unsigned long long build_type_erasure();
export unsigned long long build_polymorphic();
export unsigned long long build_reflection();
export unsigned long long cycle_direct();
export unsigned long long cycle_traditional_polymorphism();
export unsigned long long cycle_type_erasure();
export unsigned long long cycle_std_polymorphic();
export unsigned long long cycle_reflection_type_erasure();
