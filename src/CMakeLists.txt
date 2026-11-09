add_library(shapes
    Traditional_OOP/Shape.hpp
    Traditional_OOP/Circle.hpp
    Traditional_OOP/Triangle.hpp
    Traditional_OOP/Rectangle.hpp
    Traditional_OOP/Circle.cpp
    Traditional_OOP/Triangle.cpp
    Traditional_OOP/Rectangle.cpp
)

target_include_directories(shapes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(shapes PUBLIC cxx_std_26)

add_executable(draw_shapes main.cpp)
target_link_libraries(draw_shapes PRIVATE shapes)
