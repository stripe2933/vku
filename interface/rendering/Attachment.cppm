/** @file rendering/Attachment.cppm
 */

module;

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:rendering.Attachment;

import std;
export import :images.Image;

namespace vku {
    export struct Attachment {
        Image image;
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView view;
    };

    export struct SwapchainAttachment {
        std::vector<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView> views;
    };
}