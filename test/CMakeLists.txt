# TODO: due to the current Clang C++20 module and libstdc++ compatibility, the test is disabled. Enable the test when
#  it available.
if (NOT LINUX)
    add_executable(execute_hierarchical_commands execute_hierarchical_commands.cpp)
    target_link_libraries(execute_hierarchical_commands PRIVATE vku::vku)
    add_test(NAME execute_hierarchical_commands COMMAND execute_hierarchical_commands)
endif()

add_executable(get_mip_view_create_infos get_mip_view_create_infos.cpp)
target_link_libraries(get_mip_view_create_infos PRIVATE vku::vku)
target_compile_definitions(get_mip_view_create_infos PRIVATE
    COMPILED_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders/get_mip_view_create_infos"
)
add_test(NAME get_mip_view_create_infos COMMAND get_mip_view_create_infos)

add_executable(pool_sizes pool_sizes.cpp)
target_link_libraries(pool_sizes PRIVATE vku::vku)
add_test(NAME pool_sizes COMMAND pool_sizes)

add_subdirectory(msaa-triangle)
add_subdirectory(triangle)
add_subdirectory(swapchain-msaa-triangle)
add_subdirectory(swapchain-triangle)