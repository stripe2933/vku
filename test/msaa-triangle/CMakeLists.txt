add_executable(msaa-triangle main.cpp)
target_link_libraries(msaa-triangle PRIVATE vku::vku)
target_compile_definitions(msaa-triangle PRIVATE
    COMPILED_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)
add_test(NAME msaa-triangle COMMAND msaa-triangle)
set_target_properties(msaa-triangle PROPERTIES CXX_MODULE_STD 1)