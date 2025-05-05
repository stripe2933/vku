cmake_minimum_required(VERSION 3.30)

# Enable CMake's experimental standard library module support.
if (CMAKE_VERSION VERSION_GREATER_EQUAL "4.0")
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")
else()
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
endif()

project(vku
    LANGUAGES CXX
    DESCRIPTION "A set of Vulkan utilities based on RAII idiom and Modern C++ features."
    VERSION 1.4.304
)

# ----------------
# Project options.
# ----------------

option(VKU_USE_SHADERC "Add runtime GLSL compilation feature by shaderc.")
option(VKU_DEFAULT_DYNAMIC_DISPATCHER "Use the vk::DispatchLoaderDynamic as the default dispatcher.")
option(VKU_ENABLE_TEST "Enable the test targets.")

# ----------------
# External dependencies.
# ----------------

find_package(Vulkan 1.4.304 REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(VulkanMemoryAllocator-Hpp CONFIG REQUIRED)

if (VKU_USE_SHADERC)
    find_package(Vulkan COMPONENTS shaderc_combined REQUIRED)
endif()

# ----------------
# Project targets.
# ----------------

add_library(vku)
target_sources(vku PUBLIC
    FILE_SET CXX_MODULES
    FILES
        ${Vulkan_INCLUDE_DIRS}/vulkan/vulkan.cppm
        extlibs/module-ports/vk_mem_alloc.cppm
        interface/mod.cppm
        interface/buffers/mod.cppm
        interface/buffers/AllocatedBuffer.cppm
        interface/buffers/Buffer.cppm
        interface/buffers/MappedBuffer.cppm
        interface/commands.cppm
        interface/constants.cppm
        interface/debugging.cppm
        interface/descriptors/mod.cppm
        interface/descriptors/DescriptorSetLayout.cppm
        interface/descriptors/DescriptorSet.cppm
        interface/descriptors/PoolSizes.cppm
        interface/details/concepts.cppm
        interface/details/container/OnDemandCounterStorage.cppm
        interface/details/functional.cppm
        interface/details/to_string.cppm
        interface/details/tuple.cppm
        interface/Gpu.cppm
        interface/images/mod.cppm
        interface/images/AllocatedImage.cppm
        interface/images/Image.cppm
        interface/pipelines/mod.cppm
        interface/pipelines/Shader.cppm
        interface/queue.cppm
        interface/rendering/mod.cppm
        interface/rendering/Attachment.cppm
        interface/rendering/AttachmentGroup.cppm
        interface/rendering/AttachmentGroupBase.cppm
        interface/rendering/MultisampleAttachment.cppm
        interface/rendering/MultisampleAttachmentGroup.cppm
        interface/utils/mod.cppm
        interface/utils/RefHolder.cppm
)
target_compile_features(vku PUBLIC cxx_std_23)
target_include_directories(vku PRIVATE include)
target_link_libraries(vku PUBLIC
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
    VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
    $<$<BOOL:${VKU_USE_SHADERC}>:Vulkan::shaderc_combined>
)
target_compile_definitions(vku PUBLIC
    $<$<BOOL:${VKU_USE_SHADERC}>:VKU_USE_SHADERC>
    $<$<BOOL:${MSVC}>:VULKAN_HPP_NO_SMART_HANDLE> # See https://github.com/KhronosGroup/Vulkan-Hpp/blob/main/README.md#c20-named-module for the details.
    $<$<PLATFORM_ID:Windows>:VK_USE_PLATFORM_WIN32_KHR>
    $<$<PLATFORM_ID:Darwin>:VK_USE_PLATFORM_METAL_EXT VK_ENABLE_BETA_EXTENSIONS> # For VK_KHR_portability_subset availability.
    $<$<BOOL:${VKU_DEFAULT_DYNAMIC_DISPATCHER}>:VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VMA_STATIC_VULKAN_FUNCTIONS=0 VMA_DYNAMIC_VULKAN_FUNCTIONS=1>
)
set_target_properties(vku PROPERTIES 
    CXX_MODULE_STD 1
    PRIVATE_HEADER include/lifetimebound.hpp
)

# For usage consistency with FetchContent and find_package.
add_library(${PROJECT_NAME}::vku ALIAS vku)

# --------------------
# Tests.
# --------------------

if (VKU_ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

# --------------------
# Installation.
# --------------------

include(GNUInstallDirs)
install(
    TARGETS vku
    EXPORT "${PROJECT_NAME}Targets"
    FILE_SET CXX_MODULES DESTINATION module/${PROJECT_NAME}
)
install(
    EXPORT "${PROJECT_NAME}Targets"
    DESTINATION cmake/${PROJECT_NAME}
    NAMESPACE ${PROJECT_NAME}::
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY ExactVersion
)
configure_package_config_file(
    cmake/config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION cmake/${PROJECT_NAME}
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION cmake/${PROJECT_NAME}
)