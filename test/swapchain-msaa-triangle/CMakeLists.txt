add_executable(swapchain-msaa-triangle main.cpp)
target_link_libraries(swapchain-msaa-triangle PRIVATE vku::vku)
target_compile_definitions(swapchain-msaa-triangle PRIVATE
    COMPILED_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)
add_test(NAME swapchain-msaa-triangle COMMAND swapchain-msaa-triangle)
set_target_properties(swapchain-msaa-triangle PROPERTIES CXX_MODULE_STD 1)