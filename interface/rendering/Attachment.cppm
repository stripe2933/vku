module;

#ifndef VKU_USE_STD_MODULE
#include <compare>
#endif

export module vku:rendering.Attachment;

#if defined(VKU_USE_STD_MODULE) && defined(_MSC_VER)
import std;
#endif
export import vulkan_hpp;
export import :images.Image;

namespace vku {
    export struct Attachment {
        Image image;
        vk::raii::ImageView view;
    };
}