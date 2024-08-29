module;

#include <cassert>
#ifndef VKU_USE_STD_MODULE
#include <algorithm>
#include <optional>
#include <ranges>
#include <span>
#include <vector>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:rendering.AttachmentGroup;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import :rendering.Attachment;
import :rendering.AttachmentGroupBase;
import :utils.RefHolder;

// #define VMA_HPP_NAMESPACE to vma, if not defined.
#ifndef VMA_HPP_NAMESPACE
#define VMA_HPP_NAMESPACE vma
#endif

export namespace vku {
    struct AttachmentGroup : AttachmentGroupBase {
        struct ColorAttachmentInfo {
            VULKAN_HPP_NAMESPACE::AttachmentLoadOp loadOp;
            VULKAN_HPP_NAMESPACE::AttachmentStoreOp storeOp;
            VULKAN_HPP_NAMESPACE::ClearColorValue clearValue;
        };

        struct DepthStencilAttachmentInfo {
            VULKAN_HPP_NAMESPACE::AttachmentLoadOp loadOp;
            VULKAN_HPP_NAMESPACE::AttachmentStoreOp storeOp;
            VULKAN_HPP_NAMESPACE::ClearDepthStencilValue clearValue;
        };

        std::vector<Attachment> colorAttachments;
        std::optional<Attachment> depthStencilAttachment;

        explicit AttachmentGroup(const VULKAN_HPP_NAMESPACE::Extent2D &extent);
        AttachmentGroup(const AttachmentGroup&) = delete;
        AttachmentGroup(AttachmentGroup&&) noexcept = default;
        auto operator=(const AttachmentGroup&) -> AttachmentGroup& = delete;
        auto operator=(AttachmentGroup&&) noexcept -> AttachmentGroup& = default;
        ~AttachmentGroup() override = default;

        auto addColorAttachment(const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device, const Image &image, VULKAN_HPP_NAMESPACE::Format viewFormat = {}) -> const Attachment&;
        auto addColorAttachment(const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device, const Image &image, const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &viewCreateInfo) -> const Attachment&;
        auto setDepthStencilAttachment(const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device, const Image &image, VULKAN_HPP_NAMESPACE::Format viewFormat = {}) -> const Attachment&;
        auto setDepthStencilAttachment(const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device, const Image &image, const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &viewCreateInfo) -> const Attachment&;

        [[nodiscard]] auto createColorImage(
            VMA_HPP_NAMESPACE::Allocator allocator,
            VULKAN_HPP_NAMESPACE::Format format,
            VULKAN_HPP_NAMESPACE::ImageUsageFlags usage = VULKAN_HPP_NAMESPACE::ImageUsageFlagBits::eColorAttachment,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = { {}, VMA_HPP_NAMESPACE::MemoryUsage::eAutoPreferDevice }
        ) const -> AllocatedImage;

        [[nodiscard]] auto createDepthStencilImage(
            VMA_HPP_NAMESPACE::Allocator allocator,
            VULKAN_HPP_NAMESPACE::Format format,
            VULKAN_HPP_NAMESPACE::ImageUsageFlags usage = VULKAN_HPP_NAMESPACE::ImageUsageFlagBits::eDepthStencilAttachment | VULKAN_HPP_NAMESPACE::ImageUsageFlagBits::eTransientAttachment,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = { {}, VMA_HPP_NAMESPACE::MemoryUsage::eAutoPreferDevice, {}, VULKAN_HPP_NAMESPACE::MemoryPropertyFlagBits::eLazilyAllocated }
        ) const -> AllocatedImage;

        [[nodiscard]] auto getRenderingInfo(
            std::span<const ColorAttachmentInfo> colorAttachmentInfos
        ) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>>;

        [[nodiscard]] auto getRenderingInfo(
            std::span<const ColorAttachmentInfo> colorAttachmentInfos,
            const DepthStencilAttachmentInfo &depthStencilAttachmentInfo
        ) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>>;
    };
}

// --------------------
// Implementations.
// --------------------

vku::AttachmentGroup::AttachmentGroup(
    const VULKAN_HPP_NAMESPACE::Extent2D &extent
) : AttachmentGroupBase { extent } { }

auto vku::AttachmentGroup::addColorAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &image,
    VULKAN_HPP_NAMESPACE::Format viewFormat
) -> const Attachment & {
    return addColorAttachment(device, image, VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
        {},
        image,
        VULKAN_HPP_NAMESPACE::ImageViewType::e2D,
        viewFormat == VULKAN_HPP_NAMESPACE::Format::eUndefined ? image.format : viewFormat,
        {},
        { VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
    });
}

auto vku::AttachmentGroup::addColorAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &image,
    const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &viewCreateInfo
) -> const Attachment & {
    return colorAttachments.emplace_back(image, VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView { device, viewCreateInfo });
}

auto vku::AttachmentGroup::createColorImage(
    VMA_HPP_NAMESPACE::Allocator allocator,
    VULKAN_HPP_NAMESPACE::Format format,
    VULKAN_HPP_NAMESPACE::ImageUsageFlags usage,
    const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
) const -> AllocatedImage {
    return createAttachmentImage(
        allocator,
        format,
        VULKAN_HPP_NAMESPACE::SampleCountFlagBits::e1,
        usage,
        allocationCreateInfo);
}

auto vku::AttachmentGroup::setDepthStencilAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &image,
    VULKAN_HPP_NAMESPACE::Format viewFormat
) -> const Attachment & {
    if (viewFormat == VULKAN_HPP_NAMESPACE::Format::eUndefined) {
        viewFormat = image.format;
    }

    return setDepthStencilAttachment(device, image, VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
        {},
        image,
        VULKAN_HPP_NAMESPACE::ImageViewType::e2D,
        viewFormat,
        {},
        { Image::inferAspectFlags(viewFormat), 0, 1, 0, 1 },
    });
}

auto vku::AttachmentGroup::setDepthStencilAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &image,
    const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &viewCreateInfo
) -> const Attachment & {
    return depthStencilAttachment.emplace(image, VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView { device, viewCreateInfo });
}

auto vku::AttachmentGroup::createDepthStencilImage(
    VMA_HPP_NAMESPACE::Allocator allocator,
    VULKAN_HPP_NAMESPACE::Format format,
    VULKAN_HPP_NAMESPACE::ImageUsageFlags usage,
    const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
) const -> AllocatedImage {
    return createAttachmentImage(
        allocator,
        format,
        VULKAN_HPP_NAMESPACE::SampleCountFlagBits::e1,
        usage,
        allocationCreateInfo);
}

auto vku::AttachmentGroup::getRenderingInfo(
    std::span<const ColorAttachmentInfo> colorAttachmentInfos
) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(!depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");

    std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> renderingAttachmentInfos;
    renderingAttachmentInfos.reserve(colorAttachmentInfos.size());
    for (const auto &[attachment, info] : std::views::zip(colorAttachments, colorAttachmentInfos)) {
        renderingAttachmentInfos.push_back({
            *attachment.view, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
            {}, {}, {},
            info.loadOp, info.storeOp, info.clearValue,
        });
    }

    return {
        [this](std::span<const VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> colorAttachmentInfos) {
            return VULKAN_HPP_NAMESPACE::RenderingInfo {
                {},
                { { 0, 0 }, extent },
                1,
                {},
                colorAttachmentInfos,
            };
        },
        std::move(renderingAttachmentInfos),
    };
}

auto vku::AttachmentGroup::getRenderingInfo(
    std::span<const ColorAttachmentInfo> colorAttachmentInfos,
    const DepthStencilAttachmentInfo &depthStencilAttachmentInfo
) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");

    std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> renderingAttachmentInfos;
    renderingAttachmentInfos.reserve(colorAttachmentInfos.size() + 1);
    for (const auto &[attachment, info] : std::views::zip(colorAttachments, colorAttachmentInfos)) {
        renderingAttachmentInfos.push_back({
            *attachment.view, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
            {}, {}, {},
            info.loadOp, info.storeOp, info.clearValue,
        });
    }
    renderingAttachmentInfos.push_back({
        *depthStencilAttachment->view, VULKAN_HPP_NAMESPACE::ImageLayout::eDepthStencilAttachmentOptimal,
        {}, {}, {},
        depthStencilAttachmentInfo.loadOp, depthStencilAttachmentInfo.storeOp, depthStencilAttachmentInfo.clearValue,
    });

    return {
        [this](std::span<const VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> renderingAttachmentInfos) {
            return VULKAN_HPP_NAMESPACE::RenderingInfo {
                {},
                { { 0, 0 }, extent },
                1,
                {},
                unsafeProxy(renderingAttachmentInfos.subspan(0, colorAttachments.size())),
                &renderingAttachmentInfos[colorAttachments.size()],
            };
        },
        std::move(renderingAttachmentInfos),
    };
}