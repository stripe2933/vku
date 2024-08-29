module;

#ifndef VKU_USE_STD_MODULE
#include <concepts>
#include <forward_list>
#include <utility>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:rendering.AttachmentGroupBase;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vk_mem_alloc_hpp;
export import vulkan_hpp;
export import :images.AllocatedImage;
export import :utils;

// #define VMA_HPP_NAMESPACE to vma, if not defined.
#ifndef VMA_HPP_NAMESPACE
#define VMA_HPP_NAMESPACE vma
#endif

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

namespace vku {
    export class AttachmentGroupBase {
    public:
        VULKAN_HPP_NAMESPACE::Extent2D extent;

        explicit AttachmentGroupBase(
            const VULKAN_HPP_NAMESPACE::Extent2D &extent
        );

        AttachmentGroupBase(const AttachmentGroupBase&) = delete;
        AttachmentGroupBase(AttachmentGroupBase&&) noexcept = default;
        auto operator=(const AttachmentGroupBase&) -> AttachmentGroupBase& = delete;
        auto operator=(AttachmentGroupBase&&) noexcept -> AttachmentGroupBase& = default;
        virtual ~AttachmentGroupBase() = default;

        [[nodiscard]] auto storeImage(AllocatedImage &&image) -> const AllocatedImage&;

        [[nodiscard]]
        [[deprecated("Use vku::toViewport instead.")]]
        auto getViewport(
            bool negativeHeight = false
        ) const noexcept -> VULKAN_HPP_NAMESPACE::Viewport {
            return toViewport(extent, negativeHeight);
        }

        [[nodiscard]]
        [[deprecated("Use vk::Rect2D instead.")]]
        auto getScissor() const noexcept -> VULKAN_HPP_NAMESPACE::Rect2D {
            return { { 0, 0 }, extent };
        }

    protected:
        std::forward_list<AllocatedImage> storedImage;

        [[nodiscard]] auto createAttachmentImage(
            VMA_HPP_NAMESPACE::Allocator allocator,
            VULKAN_HPP_NAMESPACE::Format format,
            VULKAN_HPP_NAMESPACE::SampleCountFlagBits sampleCount,
            VULKAN_HPP_NAMESPACE::ImageUsageFlags usage,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
        ) const -> AllocatedImage;
    };
}

// --------------------
// Implementations.
// --------------------

vku::AttachmentGroupBase::AttachmentGroupBase(
    const VULKAN_HPP_NAMESPACE::Extent2D &extent
) : extent { extent } { }

auto vku::AttachmentGroupBase::storeImage(
    AllocatedImage &&image
) -> const AllocatedImage& {
    return storedImage.emplace_front(std::move(image));
}

auto vku::AttachmentGroupBase::createAttachmentImage(
    VMA_HPP_NAMESPACE::Allocator allocator,
    VULKAN_HPP_NAMESPACE::Format format,
    VULKAN_HPP_NAMESPACE::SampleCountFlagBits sampleCount,
    VULKAN_HPP_NAMESPACE::ImageUsageFlags usage,
    const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
) const -> AllocatedImage {
    return { allocator, VULKAN_HPP_NAMESPACE::ImageCreateInfo {
        {},
        VULKAN_HPP_NAMESPACE::ImageType::e2D,
        format,
        VULKAN_HPP_NAMESPACE::Extent3D { extent, 1 },
        1, 1,
        sampleCount,
        VULKAN_HPP_NAMESPACE::ImageTiling::eOptimal,
        usage,
    }, allocationCreateInfo };
}