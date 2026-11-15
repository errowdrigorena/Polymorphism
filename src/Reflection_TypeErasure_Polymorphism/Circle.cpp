module reflection_type_erasure:circle_impl;

import std;
import :circle;

void Circle::draw()
{
    std::cout << "Circle(" << radius << ")\n";
}

float Circle::area()
{
    return std::numbers::pi_v<float> * radius * radius;
}

float Circle::perimeter()
{
    return 2.0f * std::numbers::pi_v<float> * radius;
}
