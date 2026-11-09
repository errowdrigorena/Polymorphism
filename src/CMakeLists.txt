add_library(shapes)

target_sources(shapes
    PUBLIC
        FILE_SET CXX_MODULES FILES
            Traditional_OOP/Shape.cppm
            Traditional_OOP/Circle.cppm
            Traditional_OOP/Triangle.cppm
            Traditional_OOP/Rectangle.cppm
            Traditional_OOP/shapes.cppm
)

target_compile_features(shapes PUBLIC cxx_std_26)

add_library(type_erasure)
target_sources(type_erasure
    PUBLIC
        FILE_SET CXX_MODULES FILES
            TypeErasureOOP/drawable.cppm
            TypeErasureOOP/shape.cppm
            TypeErasureOOP/type_erasure.cppm
)
target_compile_features(type_erasure PUBLIC cxx_std_26)

add_executable(draw_shapes main.cpp)
target_link_libraries(draw_shapes PRIVATE shapes)
