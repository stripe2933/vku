module;

#ifndef VKU_USE_STD_MODULE
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#endif

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
        vk::Extent2D extent;

        explicit AttachmentGroupBase(
            const vk::Extent2D &extent
        );

        AttachmentGroupBase(const AttachmentGroupBase&) = delete;
        AttachmentGroupBase(AttachmentGroupBase&&) noexcept = default;
        auto operator=(const AttachmentGroupBase&) -> AttachmentGroupBase& = delete;
        auto operator=(AttachmentGroupBase&&) noexcept -> AttachmentGroupBase& = default;
        virtual ~AttachmentGroupBase() = default;

        [[nodiscard]] auto storeImage(std::unique_ptr<AllocatedImage> image) -> const AllocatedImage&;
        [[nodiscard]] auto storeImage(
            std::derived_from<AllocatedImage> auto &&image
        ) -> const AllocatedImage& {
            return *storedImage.emplace_back(std::make_unique<std::remove_cvref_t<decltype(image)>>(FWD(image)));
        }

        [[nodiscard]]
        [[deprecated("Use vku::toViewport instead.")]]
        auto getViewport(
            bool negativeHeight = false
        ) const noexcept -> vk::Viewport {
            return toViewport(extent, negativeHeight);
        }

        [[nodiscard]]
        [[deprecated("Use vk::Rect2D instead.")]]
        auto getScissor() const noexcept -> vk::Rect2D {
            return { { 0, 0 }, extent };
        }

    protected:
        std::vector<std::unique_ptr<AllocatedImage>> storedImage;

        [[nodiscard]] auto createAttachmentImage(
            VMA_HPP_NAMESPACE::Allocator allocator,
            vk::Format format,
            vk::SampleCountFlagBits sampleCount,
            vk::ImageUsageFlags usage,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
        ) const -> AllocatedImage;
    };
}

// --------------------
// Implementations.
// --------------------

vku::AttachmentGroupBase::AttachmentGroupBase(
    const vk::Extent2D &extent
) : extent { extent } { }

auto vku::AttachmentGroupBase::storeImage(
    std::unique_ptr<AllocatedImage> image
) -> const AllocatedImage& {
    return *storedImage.emplace_back(std::move(image));
}

auto vku::AttachmentGroupBase::createAttachmentImage(
    VMA_HPP_NAMESPACE::Allocator allocator,
    vk::Format format,
    vk::SampleCountFlagBits sampleCount,
    vk::ImageUsageFlags usage,
    const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
) const -> AllocatedImage {
    return { allocator, vk::ImageCreateInfo {
        {},
        vk::ImageType::e2D,
        format,
        vk::Extent3D { extent, 1 },
        1, 1,
        sampleCount,
        vk::ImageTiling::eOptimal,
        usage,
    }, allocationCreateInfo };
}