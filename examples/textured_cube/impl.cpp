#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#ifdef VKU_EXAMPLE_USE_MODULE
#include <vulkan/vulkan_hpp_macros.hpp>

import vulkan_hpp;
#else
#define VMA_IMPLEMENTATION
#define VKU_IMPLEMENTATION
#include <vku.hpp>
#endif

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif