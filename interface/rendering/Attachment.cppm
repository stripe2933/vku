module;

#ifndef VKU_USE_STD_MODULE
#include <vector>
#ifdef _MSC_VER
#include <compare>
#endif
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:rendering.Attachment;

#if defined(VKU_USE_STD_MODULE) && defined(_MSC_VER)
import std;
#endif
export import :images.Image;

namespace vku {
    export struct Attachment {
        Image image;
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView view;
    };
}