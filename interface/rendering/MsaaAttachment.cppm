module;

#ifndef VKU_USE_STD_MODULE
#include <compare>
#endif

export module vku:rendering.MsaaAttachment;

export import vulkan_hpp;
export import :images.Image;

namespace vku {
    export struct MsaaAttachment {
        Image image;
        vk::raii::ImageView view;
        Image resolveImage;
        vk::raii::ImageView resolveView;
    };
}