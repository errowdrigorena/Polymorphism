export module reflection_type_erasure:circle;

export struct Circle {
    float radius{};

    void draw();
    float area();
    float perimeter();
};
