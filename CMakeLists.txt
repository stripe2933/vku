cmake_minimum_required(VERSION 3.28)

if (VKU_USE_STD_MODULE)
    # Enable CMake's experimental standard library module support.
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
endif()

project(vku
    LANGUAGES CXX
    DESCRIPTION "Vulkan simplifications."
    VERSION 0.1.0
)

set(CMAKE_CXX_STANDARD 23)
if (VKU_USE_STD_MODULE)
    set(CMAKE_CXX_MODULE_STD 1)
endif()

# ----------------
# Project options.
# ----------------

option(VKU_USE_STD_MODULE "Use the standard library module for compilation.")

# ----------------
# External dependencies.
# ----------------

find_package(Vulkan 1.3.256 REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(VulkanMemoryAllocator-Hpp CONFIG REQUIRED)

# ----------------
# Module configurations for the external dependencies.
# ----------------

if (NOT TARGET Vulkan-Hpp_module)
    add_library(Vulkan-Hpp_module)
    target_sources(Vulkan-Hpp_module
        PUBLIC
            FILE_SET CXX_MODULES
            BASE_DIRS ${Vulkan_INCLUDE_DIRS}
            FILES ${Vulkan_INCLUDE_DIRS}/vulkan/vulkan.cppm
    )
    target_compile_features(Vulkan-Hpp_module PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
    target_link_libraries(Vulkan-Hpp_module PUBLIC Vulkan::Vulkan)

    add_library(Vulkan-Hpp::module ALIAS Vulkan-Hpp_module)
endif()

target_compile_definitions(Vulkan-Hpp_module PUBLIC
    $<$<BOOL:${MSVC}>:VULKAN_HPP_NO_SMART_HANDLE> # See https://github.com/KhronosGroup/Vulkan-Hpp/blob/main/README.md#c20-named-module for the details.
)

if (NOT TARGET VulkanMemoryAllocator-Hpp_module)
    add_library(VulkanMemoryAllocator-Hpp_module)
    target_sources(VulkanMemoryAllocator-Hpp_module
        PUBLIC
            FILE_SET CXX_MODULES
            FILES extlibs/module-ports/vk_mem_alloc.cppm
    )
    target_compile_features(VulkanMemoryAllocator-Hpp_module PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
    target_link_libraries(VulkanMemoryAllocator-Hpp_module PUBLIC
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
        VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
    )

    add_library(VulkanMemoryAllocator-Hpp::module ALIAS VulkanMemoryAllocator-Hpp_module)
endif()

# ----------------
# Project targets.
# ----------------

add_library(vku)
target_sources(vku
    PUBLIC
        FILE_SET CXX_MODULES
        FILES
            interface/mod.cppm
            interface/buffers/mod.cppm
            interface/buffers/AllocatedBuffer.cppm
            interface/buffers/Buffer.cppm
            interface/buffers/MappedBuffer.cppm
            interface/commands.cppm
            interface/descriptors/mod.cppm
            interface/descriptors/DescriptorSetLayouts.cppm
            interface/descriptors/DescriptorSets.cppm
            interface/descriptors/PoolSizes.cppm
            interface/details/mod.cppm
            interface/details/concepts.cppm
            interface/details/ranges.cppm
            interface/images/mod.cppm
            interface/images/AllocatedImage.cppm
            interface/images/Image.cppm
            interface/pipelines/mod.cppm
            interface/pipelines/Shader.cppm
            interface/rendering/mod.cppm
            interface/rendering/Attachment.cppm
            interface/rendering/AttachmentGroup.cppm
            interface/rendering/AttachmentGroupBase.cppm
            interface/rendering/MsaaAttachment.cppm
            interface/rendering/MsaaAttachmentGroup.cppm
            interface/utils/mod.cppm
            interface/utils/RefHolder.cppm
)
target_compile_features(vku PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(vku
    PUBLIC
        Vulkan-Hpp::module
        VulkanMemoryAllocator-Hpp::module
)
target_compile_definitions(vku PRIVATE
    $<$<BOOL:${VKU_USE_STD_MODULE}>:VKU_USE_STD_MODULE>
)

# For usage consistency with FetchContent and find_package.
add_library(vku::vku ALIAS vku)

# --------------------
# Installation.
# --------------------

include(GNUInstallDirs)
install(
    TARGETS Vulkan-Hpp_module VulkanMemoryAllocator-Hpp_module vku
    EXPORT "${PROJECT_NAME}Targets"
    FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
    EXPORT "${PROJECT_NAME}Targets"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    NAMESPACE ${PROJECT_NAME}::
    CXX_MODULES_DIRECTORY .
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion # Change this to SameMinorVersion when reach to the version 1.0.
)
configure_package_config_file(cmake/config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)