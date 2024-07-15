module;

#include <cassert>
#ifndef VKU_USE_STD_MODULE
#include <optional>
#include <ranges>
#include <span>
#include <vector>
#endif

export module vku:rendering.MsaaAttachmentGroup;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
import :details;
export import :images.AllocatedImage;
export import :rendering.Attachment;
import :rendering.AttachmentGroupBase;
export import :rendering.MsaaAttachment;
import :utils.RefHolder;

namespace vku {
    export struct MsaaAttachmentGroup : AttachmentGroupBase {
        struct ColorAttachmentInfo {
            vk::AttachmentLoadOp loadOp;
            vk::AttachmentStoreOp storeOp;
            vk::ClearColorValue clearValue;
            vk::ResolveModeFlagBits resolveMode = vk::ResolveModeFlagBits::eAverage;
        };

        struct DepthStencilAttachmentInfo {
            vk::AttachmentLoadOp loadOp;
            vk::AttachmentStoreOp storeOp;
            vk::ClearDepthStencilValue clearValue;
        };

        vk::SampleCountFlagBits sampleCount;
        std::vector<MsaaAttachment> colorAttachments;
        std::optional<Attachment> depthStencilAttachment;

        explicit MsaaAttachmentGroup(const vk::Extent2D &extent, vk::SampleCountFlagBits sampleCount);
        MsaaAttachmentGroup(const MsaaAttachmentGroup&) = delete;
        MsaaAttachmentGroup(MsaaAttachmentGroup&&) noexcept = default;
        auto operator=(const MsaaAttachmentGroup&) -> MsaaAttachmentGroup& = delete;
        auto operator=(MsaaAttachmentGroup&&) noexcept -> MsaaAttachmentGroup& = default;
        ~MsaaAttachmentGroup() override = default;

        auto addColorAttachment(const vk::raii::Device &device, const Image &image, const Image &resolveImage, vk::Format viewFormat = {}) -> const MsaaAttachment&;
        auto addColorAttachment(const vk::raii::Device &device, const Image &image, const Image &resolveImage, const vk::ImageViewCreateInfo &viewCreateInfo, const vk::ImageViewCreateInfo &resolveViewCreateInfo) -> const MsaaAttachment&;
        auto setDepthStencilAttachment(const vk::raii::Device &device, const Image &image, vk::Format viewFormat = {}) -> const Attachment&;
        auto setDepthStencilAttachment(const vk::raii::Device &device, const Image &image, const vk::ImageViewCreateInfo &viewCreateInfo) -> const Attachment&;

        [[nodiscard]] auto createColorImage(
            vma::Allocator allocator,
            vk::Format format,
            vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
            const vma::AllocationCreateInfo &allocationCreateInfo = { {}, vma::MemoryUsage::eAutoPreferDevice, {}, vk::MemoryPropertyFlagBits::eLazilyAllocated }
        ) const -> AllocatedImage;

        [[nodiscard]] auto createResolveImage(
            vma::Allocator allocator,
            vk::Format format,
            vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment,
            const vma::AllocationCreateInfo &allocationCreateInfo = { {}, vma::MemoryUsage::eAutoPreferDevice }
        ) const -> AllocatedImage;

        [[nodiscard]] auto createDepthStencilImage(
            vma::Allocator allocator,
            vk::Format format,
            vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
            const vma::AllocationCreateInfo &allocationCreateInfo = { {}, vma::MemoryUsage::eAutoPreferDevice, {}, vk::MemoryPropertyFlagBits::eLazilyAllocated }
        ) const -> AllocatedImage;

        [[nodiscard]] auto getRenderingInfo(
            std::span<const ColorAttachmentInfo> colorAttachmentInfos
        ) const -> RefHolder<vk::RenderingInfo, std::vector<vk::RenderingAttachmentInfo>>;

        [[nodiscard]] auto getRenderingInfo(
            std::span<const ColorAttachmentInfo> colorAttachmentInfos,
            const DepthStencilAttachmentInfo &depthStencilAttachmentInfo
        ) const -> RefHolder<vk::RenderingInfo, std::vector<vk::RenderingAttachmentInfo>, vk::RenderingAttachmentInfo>;
    };
}

// --------------------
// Implementations.
// --------------------

vku::MsaaAttachmentGroup::MsaaAttachmentGroup(
    const vk::Extent2D &extent,
    vk::SampleCountFlagBits sampleCount
) : AttachmentGroupBase { extent },
    sampleCount { sampleCount } { }

auto vku::MsaaAttachmentGroup::addColorAttachment(
    const vk::raii::Device &device,
    const Image &image,
    const Image &resolveImage,
    vk::Format viewFormat
) -> const MsaaAttachment& {
    return addColorAttachment(
        device,
        image,
        resolveImage,
        vk::ImageViewCreateInfo {
            {},
            image,
            vk::ImageViewType::e2D,
            viewFormat == vk::Format::eUndefined ? image.format : viewFormat,
            {},
            { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
        },
        vk::ImageViewCreateInfo {
            {},
            resolveImage,
            vk::ImageViewType::e2D,
            viewFormat == vk::Format::eUndefined ? resolveImage.format : viewFormat,
            {},
            { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
        });
}

auto vku::MsaaAttachmentGroup::addColorAttachment(
    const vk::raii::Device &device,
    const Image &image,
    const Image &resolveImage,
    const vk::ImageViewCreateInfo &viewCreateInfo,
    const vk::ImageViewCreateInfo &resolveViewCreateInfo
) -> const MsaaAttachment & {
    return colorAttachments.emplace_back(
        image,
        vk::raii::ImageView { device, viewCreateInfo },
        resolveImage,
        vk::raii::ImageView { device, resolveViewCreateInfo });
}

auto vku::MsaaAttachmentGroup::setDepthStencilAttachment(
    const vk::raii::Device &device,
    const Image &image,
    vk::Format viewFormat
) -> const Attachment& {
    if (viewFormat == vk::Format::eUndefined) {
        viewFormat = image.format;
    }
    return setDepthStencilAttachment(device, image, vk::ImageViewCreateInfo {
        {},
        image,
        vk::ImageViewType::e2D,
        viewFormat,
        {},
        { Image::inferAspectFlags(viewFormat), 0, 1, 0, 1 },
    });
}

auto vku::MsaaAttachmentGroup::setDepthStencilAttachment(
    const vk::raii::Device &device,
    const Image &image,
    const vk::ImageViewCreateInfo &viewCreateInfo
) -> const Attachment & {
    return depthStencilAttachment.emplace(image, vk::raii::ImageView { device, viewCreateInfo });
}

auto vku::MsaaAttachmentGroup::createColorImage(
    vma::Allocator allocator,
    vk::Format format,
    vk::ImageUsageFlags usage,
    const vma::AllocationCreateInfo &allocationCreateInfo
) const -> AllocatedImage {
    return createAttachmentImage(allocator, format, sampleCount, usage, allocationCreateInfo);
}

auto vku::MsaaAttachmentGroup::createResolveImage(
    vma::Allocator allocator,
    vk::Format viewFormat,
    vk::ImageUsageFlags usage,
    const vma::AllocationCreateInfo &allocationCreateInfo
) const -> AllocatedImage {
    return createAttachmentImage(allocator, viewFormat, vk::SampleCountFlagBits::e1, usage, allocationCreateInfo);
}

auto vku::MsaaAttachmentGroup::createDepthStencilImage(
    vma::Allocator allocator,
    vk::Format format,
    vk::ImageUsageFlags usage,
    const vma::AllocationCreateInfo &allocationCreateInfo
) const -> AllocatedImage {
    return createAttachmentImage(
        allocator,
        format,
        sampleCount,
        usage,
        allocationCreateInfo);
}

auto vku::MsaaAttachmentGroup::getRenderingInfo(
    std::span<const ColorAttachmentInfo> colorAttachmentInfos
) const -> RefHolder<vk::RenderingInfo, std::vector<vk::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(!depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");
    return {
        [this](std::span<const vk::RenderingAttachmentInfo> colorAttachmentInfos) {
            return vk::RenderingInfo {
                {},
                { { 0, 0 }, extent },
                1,
                {},
                colorAttachmentInfos,
            };
        },
        ranges::views::zip_transform([](const MsaaAttachment &attachment, const ColorAttachmentInfo &info) {
            return vk::RenderingAttachmentInfo {
                *attachment.view, vk::ImageLayout::eColorAttachmentOptimal,
                info.resolveMode, *attachment.resolveView,
                vk::ImageLayout::eColorAttachmentOptimal,
                info.loadOp, info.storeOp, info.clearValue,
            };
        }, colorAttachments, colorAttachmentInfos) | std::ranges::to<std::vector>(),
    };
}

auto vku::MsaaAttachmentGroup::getRenderingInfo(
    std::span<const ColorAttachmentInfo> colorAttachmentInfos,
    const DepthStencilAttachmentInfo &depthStencilAttachmentInfo
) const -> RefHolder<vk::RenderingInfo, std::vector<vk::RenderingAttachmentInfo>, vk::RenderingAttachmentInfo> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");
    return {
        [this](std::span<const vk::RenderingAttachmentInfo> colorAttachmentInfos, const vk::RenderingAttachmentInfo &depthStencilAttachmentInfo) {
            return vk::RenderingInfo {
                {},
                { { 0, 0 }, extent },
                1,
                {},
                colorAttachmentInfos,
                &depthStencilAttachmentInfo,
            };
        },
        ranges::views::zip_transform([](const MsaaAttachment &attachment, const ColorAttachmentInfo &info) {
            return vk::RenderingAttachmentInfo {
                *attachment.view, vk::ImageLayout::eColorAttachmentOptimal,
                info.resolveMode, *attachment.resolveView, vk::ImageLayout::eColorAttachmentOptimal,
                info.loadOp, info.storeOp, info.clearValue,
            };
        }, colorAttachments, colorAttachmentInfos) | std::ranges::to<std::vector>(),
        vk::RenderingAttachmentInfo {
            *depthStencilAttachment->view, vk::ImageLayout::eDepthStencilAttachmentOptimal,
            {}, {}, {},
            depthStencilAttachmentInfo.loadOp, depthStencilAttachmentInfo.storeOp, depthStencilAttachmentInfo.clearValue,
        },
    };
}