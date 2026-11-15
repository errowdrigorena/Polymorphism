module reflection_type_erasure:square_impl;

import std;
import :square;

void Square::draw()
{
    std::cout << "Square(" << side << ")\n";
}

float Square::area()
{
    return side * side;
}

float Square::perimeter()
{
    return 4.0f * side;
}
