export module benchmark_work;

// This interface stays free of import std. timer.cpp imports it beside the
// Google Benchmark headers, and this GCC rejects that mix.

export constexpr unsigned long object_count = 12288;

// One width per technique. The same five names appear in every family.
export struct ApproachBytes {
    unsigned long direct;
    unsigned long traditional_polymorphism;
    unsigned long type_erasure;
    unsigned long std_polymorphic;
    unsigned long reflection_type_erasure;
};

export void prepare_benchmarks();
export ApproachBytes draw_bytes();
export ApproachBytes area_bytes();
export ApproachBytes copy_bytes();
export ApproachBytes build_bytes();
export ApproachBytes cycle_bytes();

export unsigned long long sweep_draw_direct();
export unsigned long long sweep_draw_traditional_polymorphism();
export unsigned long long sweep_draw_type_erasure();
export unsigned long long sweep_draw_std_polymorphic();
export unsigned long long sweep_draw_reflection_type_erasure();
export unsigned long long sweep_area_direct();
export unsigned long long sweep_area_traditional_polymorphism();
export unsigned long long sweep_area_type_erasure();
export unsigned long long sweep_area_std_polymorphic();
export unsigned long long sweep_area_reflection_type_erasure();
export unsigned long long copy_direct();
export unsigned long long copy_traditional_polymorphism();
export unsigned long long copy_type_erasure();
export unsigned long long copy_std_polymorphic();
export unsigned long long copy_reflection_type_erasure();
export unsigned long long build_direct();
export unsigned long long build_traditional_polymorphism();
export unsigned long long build_type_erasure();
export unsigned long long build_std_polymorphic();
export unsigned long long build_reflection_type_erasure();
export unsigned long long cycle_direct();
export unsigned long long cycle_traditional_polymorphism();
export unsigned long long cycle_type_erasure();
export unsigned long long cycle_std_polymorphic();
export unsigned long long cycle_reflection_type_erasure();
export unsigned long long cycle_direct_5();
export unsigned long long cycle_traditional_polymorphism_5();
export unsigned long long cycle_type_erasure_5();
export unsigned long long cycle_std_polymorphic_5();
export unsigned long long cycle_reflection_type_erasure_5();
export unsigned long long cycle_direct_50();
export unsigned long long cycle_traditional_polymorphism_50();
export unsigned long long cycle_type_erasure_50();
export unsigned long long cycle_std_polymorphic_50();
export unsigned long long cycle_reflection_type_erasure_50();
export unsigned long long cycle_direct_500();
export unsigned long long cycle_traditional_polymorphism_500();
export unsigned long long cycle_type_erasure_500();
export unsigned long long cycle_std_polymorphic_500();
export unsigned long long cycle_reflection_type_erasure_500();
export unsigned long long cycle_direct_5000();
export unsigned long long cycle_traditional_polymorphism_5000();
export unsigned long long cycle_type_erasure_5000();
export unsigned long long cycle_std_polymorphic_5000();
export unsigned long long cycle_reflection_type_erasure_5000();
