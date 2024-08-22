add_executable(triangle main.cpp)
target_link_libraries(triangle PRIVATE vku::vku)
target_compile_definitions(triangle PRIVATE
    COMPILED_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)
add_test(NAME triangle COMMAND triangle)