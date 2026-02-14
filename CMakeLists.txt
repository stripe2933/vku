cmake_minimum_required(VERSION 3.30)

if (VKU_BUILD_MODULE)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "4.0.3")
        set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
    elseif (CMAKE_VERSION VERSION_GREATER_EQUAL "4.0.1")
        set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")
    else()
        set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
    endif()
endif()

project(vku
    LANGUAGES CXX
    VERSION 1.4.335
)

option(VKU_DISPATCH_LOADER_DYNAMIC "Use Vulkan-Hpp dynamic dispatch loader.")
option(VKU_BUILD_MODULE "Build vku::module target with C++20 modules.")
option(VKU_BUILD_EXAMPLES "Build examples.")

find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator-hpp CONFIG REQUIRED)

# ----- vku::vku -----

add_library(vku INTERFACE)
target_compile_features(vku INTERFACE cxx_std_20)
target_include_directories(vku INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(vku INTERFACE
    Vulkan::Headers
    GPUOpen::VulkanMemoryAllocator
    unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
)

set_target_properties(vku PROPERTIES PUBLIC_HEADER include/vku.hpp)

# Usage consistency between FetchContent and find_package.
add_library(vku::vku ALIAS vku)

# ----- vku::module -----

if (VKU_BUILD_MODULE)
    get_target_property(
        VulkanMemoryAllocator_Hpp_INCLUDE_DIR
        unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
        INTERFACE_INCLUDE_DIRECTORIES
    )

    add_library(vku_module)
    target_compile_features(vku_module PUBLIC cxx_std_23)
    target_sources(vku_module PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS
            ${Vulkan_INCLUDE_DIRS}/vulkan
            ${VulkanMemoryAllocator_Hpp_INCLUDE_DIR}/vulkan-memory-allocator-hpp
            module
        FILES
            ${Vulkan_INCLUDE_DIRS}/vulkan/vulkan.cppm
            ${Vulkan_INCLUDE_DIRS}/vulkan/vulkan_video.cppm
            ${VulkanMemoryAllocator_Hpp_INCLUDE_DIR}/vulkan-memory-allocator-hpp/vk_mem_alloc.cppm
            module/vku.cppm
    )
    target_compile_definitions(vku_module
        PUBLIC
            $<$<BOOL:${VKU_DISPATCH_LOADER_DYNAMIC}>:VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VMA_STATIC_VULKAN_FUNCTIONS=0 VMA_DYNAMIC_VULKAN_FUNCTIONS=1>
        PRIVATE
            VMA_ENABLE_VULKAN_HPP_MODULE
            $<$<BOOL:${MSVC}>:VULKAN_HPP_NO_SMART_HANDLE> # See https://github.com/KhronosGroup/Vulkan-Hpp/blob/main/README.md#c20-named-module for the details.
            $<$<PLATFORM_ID:Windows>:VK_USE_PLATFORM_WIN32_KHR>
            $<$<PLATFORM_ID:Darwin>:VK_USE_PLATFORM_METAL_EXT VK_ENABLE_BETA_EXTENSIONS> # For VK_KHR_portability_subset availability.
    )
    target_link_libraries(vku_module PUBLIC vku)

    set_target_properties(vku_module PROPERTIES CXX_MODULE_STD 1)

    # Usage consistency between FetchContent and find_package.
    add_library(vku::module ALIAS vku_module)
endif()

# ----- Examples -----

if (VKU_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ----- Installation -----

install(
    TARGETS vku
    EXPORT "${PROJECT_NAME}Targets"
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