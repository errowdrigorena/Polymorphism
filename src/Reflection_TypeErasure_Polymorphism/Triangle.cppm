export module reflection_type_erasure:triangle;

export struct Triangle {
    float a{};
    float b{};
    float c{};

    void draw();
    float area();
    float perimeter();
};
