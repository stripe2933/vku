module;

#ifndef VKU_USE_STD_MODULE
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:rendering.AttachmentGroupBase;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vk_mem_alloc_hpp;
export import vulkan_hpp;
export import :images.AllocatedImage;
export import :utils.RefHolder;

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

        [[nodiscard]] auto getViewport(
            bool negativeViewport = false
        ) const noexcept -> vk::Viewport {
            return {
                0.f, negativeViewport ? static_cast<float>(extent.height) : 0.f,
                static_cast<float>(extent.width), negativeViewport ? -static_cast<float>(extent.height) : static_cast<float>(extent.height),
                0.f, 1.f,
            };
        }

        [[nodiscard]] auto getScissor() const noexcept -> vk::Rect2D {
            return { { 0, 0 }, extent };
        }

    protected:
        std::vector<std::unique_ptr<AllocatedImage>> storedImage;

        [[nodiscard]] auto createAttachmentImage(
            vma::Allocator allocator,
            vk::Format format,
            vk::SampleCountFlagBits sampleCount,
            vk::ImageUsageFlags usage,
            const vma::AllocationCreateInfo &allocationCreateInfo
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
    vma::Allocator allocator,
    vk::Format format,
    vk::SampleCountFlagBits sampleCount,
    vk::ImageUsageFlags usage,
    const vma::AllocationCreateInfo &allocationCreateInfo
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