// Importers write `import shapes;`. Each partition is one class interface.
// Circle, Triangle, and Rectangle are final leaves, and their draw() bodies
// live in the implementation units Circle.cpp, Triangle.cpp, and Rectangle.cpp.
export module shapes;

export import :shape;
export import :circle;
export import :triangle;
export import :rectangle;
