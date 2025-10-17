# ----- External dependencies -----

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(Vulkan COMPONENTS glslangValidator REQUIRED)

# ----- vku_example_textured_cube -----

add_executable(vku_example_textured_cube
    main.cpp
    impl.cpp
)
target_include_directories(vku_example_textured_cube PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(vku_example_textured_cube PRIVATE glfw glm::glm)

if (VKU_BUILD_MODULE)
    get_target_property(glm_INCLUDE_DIR glm::glm-header-only INTERFACE_INCLUDE_DIRECTORIES)

    target_sources(vku_example_textured_cube PRIVATE
        FILE_SET CXX_MODULES
        BASE_DIRS ${glm_INCLUDE_DIR}/glm
        FILES ${glm_INCLUDE_DIR}/glm/glm.cppm
    )
    target_compile_definitions(vku_example_textured_cube PRIVATE
        VKU_EXAMPLE_USE_MODULE
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_XYZW_ONLY
        GLM_GTC_INLINE_NAMESPACE
    )
    target_link_libraries(vku_example_textured_cube PRIVATE vku::module)

    set_target_properties(vku_example_textured_cube PROPERTIES CXX_MODULE_STD 1)
else()
    target_compile_definitions(vku_example_textured_cube PRIVATE
        $<$<PLATFORM_ID:Darwin>:VK_ENABLE_BETA_EXTENSIONS> # VK_KHR_portability_subset
        VULKAN_HPP_NO_SMART_HANDLE
        VULKAN_HPP_NO_TO_STRING
        $<$<BOOL:${VKU_DISPATCH_LOADER_DYNAMIC}>:VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VMA_STATIC_VULKAN_FUNCTIONS=0 VMA_DYNAMIC_VULKAN_FUNCTIONS=1>
    )
    target_link_libraries(vku_example_textured_cube PRIVATE vku::vku)
endif()

if (NOT ${VKU_DISPATCH_LOADER_DYNAMIC})
    target_link_libraries(vku_example_textured_cube PRIVATE Vulkan::Vulkan)
endif()

# ----- Shader compilation -----

set(shaders
    shaders/cube.vert
    shaders/cube.frag
)

foreach (shader IN LISTS shaders)
    add_custom_command(
        OUTPUT ${shader}.spv.h
        COMMAND Vulkan::glslangValidator -V -x ${shader} -o ${shader}.spv.h
        DEPENDS ${shader}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
    )
    target_sources(vku_example_textured_cube PRIVATE ${shader}.spv.h)
endforeach()

# ----- Resources -----

set(resources
    ../resources/texture.jpg
)

add_custom_command(
    TARGET vku_example_textured_cube POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${resources} $<TARGET_FILE_DIR:vku_example_textured_cube>
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
    COMMAND_EXPAND_LISTS
)
