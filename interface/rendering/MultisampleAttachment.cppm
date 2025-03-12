/** @file rendering/MultisampleAttachment.cppm
 */

module;

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:rendering.MultisampleAttachment;

import std;
export import :images.Image;

namespace vku {
    export struct MultisampleAttachment {
        Image multisampleImage;
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView multisampleView;
        Image image;
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView view;
    };

    export struct SwapchainMultisampleAttachment {
        Image multisampleImage;
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView multisampleView;
        std::vector<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView> views;
    };
}