add_executable(swapchain-triangle main.cpp)
target_link_libraries(swapchain-triangle PRIVATE vku::vku)
target_compile_definitions(swapchain-triangle PRIVATE
    COMPILED_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)
add_test(NAME swapchain-triangle COMMAND swapchain-triangle)