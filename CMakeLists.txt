cmake_minimum_required(VERSION 3.24)
project(vku
    LANGUAGES CXX
)

# ----------------
# CMake options.
# ----------------

if (NOT VKU_VK_VERSION)
    message(STATUS "VKU_VK_VERSION not set. Defaulting to Vulkan 1.0 (1000000).")
    set(VKU_VK_VERSION 1000000)
endif()

option(VKU_USE_GLFW "Use GLFW for window system integration.")
option(VKU_USE_GLM "Use GLM for vector/matrix mathematics. This automatically enabled when VKU_USE_GLFW ON.")
option(VKU_BUILD_EXAMPLES "Build examples.")

if (VKU_USE_GLFW)
    set(VKU_USE_GLM ON)
endif()

# ----------------
# External dependencies.
# ----------------

find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator-hpp CONFIG REQUIRED)
if (VKU_USE_GLM)
    find_package(glm CONFIG REQUIRED)
endif()
if (VKU_USE_GLFW)
    find_package(GLFW3 CONFIG REQUIRED)
endif()

# ----------------
# CMake scripts.
# ----------------

include(cmake/CompileVulkanShaders.cmake)

# ----------------
# Project libraries.
# ----------------

add_library(vku
    src/AttachmentGroup.cpp
    src/buffers.cpp
    src/images.cpp
    src/MsaaAttachmentGroup.cpp
    src/Shader.cpp
    src/pipelines.cpp
    src/rendering.cpp
)
target_compile_features(vku PUBLIC cxx_std_23)
target_include_directories(vku PRIVATE include)
target_include_directories(vku SYSTEM INTERFACE include) # Do not warn about warnings in public headers.
target_link_libraries(vku PUBLIC
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
    unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
)
target_compile_definitions(vku PUBLIC VKU_VK_VERSION=${VKU_VK_VERSION})

if (APPLE)
    target_compile_definitions(vku PUBLIC VK_ENABLE_BETA_EXTENSIONS)
endif()
target_compile_options(vku PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/wd4068> # Disable unknown pragma warning (ignore #pragma clang ...)
)

if (VKU_USE_GLFW)
    target_sources(vku PRIVATE src/GlfwWindow.cpp)
    target_link_libraries(vku PUBLIC glfw)
    target_compile_definitions(vku PUBLIC GLFW_INCLUDE_VULKAN)
endif()
if (VKU_USE_GLM)
    target_link_libraries(vku PUBLIC glm::glm)
    target_compile_definitions(vku PUBLIC VKU_USE_GLM)
endif()

# ----------------
# Examples.
# ----------------

if (VKU_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()