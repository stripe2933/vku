# ----- External dependencies -----

find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan COMPONENTS glslangValidator REQUIRED)

# ----- vku_example_triangle -----

add_executable(vku_example_triangle main.cpp impl.cpp)
target_link_libraries(vku_example_triangle PRIVATE glfw)

if (VKU_BUILD_MODULE)
    target_compile_definitions(vku_example_triangle PRIVATE VKU_EXAMPLE_USE_MODULE)
    target_link_libraries(vku_example_triangle PRIVATE vku::module)

    set_target_properties(vku_example_triangle PROPERTIES CXX_MODULE_STD 1)
else()
    target_compile_definitions(vku_example_triangle PRIVATE
        $<$<PLATFORM_ID:Darwin>:VK_ENABLE_BETA_EXTENSIONS> # VK_KHR_portability_subset
        VULKAN_HPP_NO_SETTERS
        VULKAN_HPP_NO_SMART_HANDLE
        VULKAN_HPP_NO_TO_STRING
        $<$<BOOL:${VKU_DISPATCH_LOADER_DYNAMIC}>:VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VMA_STATIC_VULKAN_FUNCTIONS=0 VMA_DYNAMIC_VULKAN_FUNCTIONS=1>
    )
    target_link_libraries(vku_example_triangle PRIVATE vku::vku)
endif()

if (NOT ${VKU_DISPATCH_LOADER_DYNAMIC})
    target_link_libraries(vku_example_triangle PRIVATE Vulkan::Vulkan)
endif()

# ----- Shader compilation -----

set(shaders
    shaders/triangle.vert
    shaders/triangle.frag
)

foreach (shader IN LISTS shaders)
    add_custom_command(
        OUTPUT ${shader}.spv.h
        COMMAND Vulkan::glslangValidator -V -x ${shader} -o ${shader}.spv.h
        DEPENDS ${shader}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
    )
    target_sources(vku_example_triangle PRIVATE ${shader}.spv.h)
endforeach()
