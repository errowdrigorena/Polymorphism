module reflection_type_erasure:triangle_impl;

import std;
import :triangle;

void Triangle::draw()
{
    std::cout << "Triangle(" << a << "," << b << "," << c << ")\n";
}

float Triangle::area()
{
    const float semi = (a + b + c) / 2.f;
    return std::sqrt(semi * (semi - a) * (semi - b) * (semi - c));
}

float Triangle::perimeter()
{
    return a + b + c;
}
