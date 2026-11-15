# GCC ships the source of `import std;`. Build that module so these libraries
# can import the standard library instead of including its headers.
execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++.modules.json
    OUTPUT_VARIABLE std_modules_json
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
file(READ "${std_modules_json}" std_modules)
string(JSON std_module_source GET "${std_modules}" modules 0 source-path)
cmake_path(GET std_module_source PARENT_PATH std_module_dir)

add_library(std_module)
target_sources(std_module
    PUBLIC
        FILE_SET CXX_MODULES BASE_DIRS "${std_module_dir}" FILES "${std_module_source}"
)
target_compile_features(std_module PUBLIC cxx_std_26)

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
target_link_libraries(shapes PUBLIC std_module)

add_library(type_erasure)
target_sources(type_erasure
    PUBLIC
        FILE_SET CXX_MODULES FILES
            TypeErasure_Polymorphism/drawable.cppm
            TypeErasure_Polymorphism/shape.cppm
            TypeErasure_Polymorphism/type_erasure.cppm
)
target_link_libraries(type_erasure PUBLIC std_module)

add_executable(draw_shapes main.cpp)
target_link_libraries(draw_shapes PRIVATE shapes)
