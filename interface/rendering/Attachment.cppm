module;

#ifndef VKU_USE_STD_MODULE
#include <compare>
#endif

export module vku:rendering.Attachment;

export import vulkan_hpp;
export import :images.Image;

namespace vku {
    export struct Attachment {
        Image image;
        vk::raii::ImageView view;
    };
}