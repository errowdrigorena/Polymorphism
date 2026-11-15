export module polymorphic;

import std;
export import shapes;

// std::polymorphic gives the Shape hierarchy value semantics. Copying a PolyShape
// copies the derived object, so the standard library writes the vtable glue.
export using PolyShape = std::polymorphic<Shape>;
