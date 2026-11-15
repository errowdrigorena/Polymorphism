export module shapes:shape;

import std;

// draw() is pure virtual: a Shape is an interface, and the dynamic type supplies
// the body. The destructor is virtual so unique_ptr<Shape> destroys that object.
// Declaring the destructor suppresses implicit moves, so the five special members
// are defaulted and the concrete types stay copyable.
export class Shape {
public:
    virtual ~Shape() = default;
    Shape() = default;
    Shape(const Shape&) = default;
    Shape& operator=(const Shape&) = default;
    Shape(Shape&&) noexcept = default;
    Shape& operator=(Shape&&) noexcept = default;

    virtual std::string draw() const = 0;
};
