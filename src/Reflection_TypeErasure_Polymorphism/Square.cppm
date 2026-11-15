export module reflection_type_erasure:square;

export struct Square {
    float side{};

    void draw();
    float area();
    float perimeter();
};
