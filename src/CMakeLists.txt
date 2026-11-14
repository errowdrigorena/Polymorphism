add_library(shapes)

target_sources(shapes
    PUBLIC
        FILE_SET CXX_MODULES FILES
            Traditional_Polymorphism/Shape.cppm
            Traditional_Polymorphism/Circle.cppm
            Traditional_Polymorphism/Triangle.cppm
            Traditional_Polymorphism/Rectangle.cppm
            Traditional_Polymorphism/shapes.cppm
    PRIVATE
        Traditional_Polymorphism/Circle.cpp
        Traditional_Polymorphism/Triangle.cpp
        Traditional_Polymorphism/Rectangle.cpp
)

target_compile_features(shapes PUBLIC cxx_std_26)

add_library(type_erasure)
target_sources(type_erasure
    PUBLIC
        FILE_SET CXX_MODULES FILES
            TypeErasure_Polymorphism/drawable.cppm
            TypeErasure_Polymorphism/shape.cppm
            TypeErasure_Polymorphism/type_erasure.cppm
)
target_compile_features(type_erasure PUBLIC cxx_std_26)

add_executable(draw_shapes main.cpp)
target_link_libraries(draw_shapes PRIVATE shapes)
