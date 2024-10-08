/** @file rendering/MsaaAttachment.cppm
 */

module;

#ifndef VKU_USE_STD_MODULE
#include <vector>
#ifdef _MSC_VER
#include <compare>
#endif
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:rendering.MsaaAttachment;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import :images.Image;

namespace vku {
    export struct MsaaAttachment {
        Image image;
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView view;
        Image resolveImage;
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView resolveView;
    };

    export struct SwapchainMsaaAttachment {
        Image image;
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView view;
        std::vector<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView> resolveViews;
    };
}