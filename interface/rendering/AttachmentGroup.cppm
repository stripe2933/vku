module;

#include <cassert>
#ifndef VKU_USE_STD_MODULE
#include <optional>
#include <ranges>
#include <span>
#include <vector>
#endif

export module vku:rendering.AttachmentGroup;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
import :details;
export import :images.AllocatedImage;
export import :rendering.Attachment;
import :rendering.AttachmentGroupBase;
import :utils.RefHolder;

export namespace vku {
    struct AttachmentGroup : AttachmentGroupBase {
        struct ColorAttachmentInfo {
            vk::AttachmentLoadOp loadOp;
            vk::AttachmentStoreOp storeOp;
            vk::ClearColorValue clearValue;
        };

        struct DepthStencilAttachmentInfo {
            vk::AttachmentLoadOp loadOp;
            vk::AttachmentStoreOp storeOp;
            vk::ClearDepthStencilValue clearValue;
        };

        std::vector<Attachment> colorAttachments;
        std::optional<Attachment> depthStencilAttachment;

        explicit AttachmentGroup(const vk::Extent2D &extent);
        AttachmentGroup(const AttachmentGroup&) = delete;
        AttachmentGroup(AttachmentGroup&&) noexcept = default;
        auto operator=(const AttachmentGroup&) -> AttachmentGroup& = delete;
        auto operator=(AttachmentGroup&&) noexcept -> AttachmentGroup& = default;
        ~AttachmentGroup() override = default;

        auto addColorAttachment(const vk::raii::Device &device, const Image &image, vk::Format viewFormat = {}) -> const Attachment&;
        auto addColorAttachment(const vk::raii::Device &device, const Image &image, const vk::ImageViewCreateInfo &viewCreateInfo) -> const Attachment&;
        auto setDepthStencilAttachment(const vk::raii::Device &device, const Image &image, vk::Format viewFormat = {}) -> const Attachment&;
        auto setDepthStencilAttachment(const vk::raii::Device &device, const Image &image, const vk::ImageViewCreateInfo &viewCreateInfo) -> const Attachment&;

        [[nodiscard]] auto createColorImage(
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

vku::AttachmentGroup::AttachmentGroup(
    const vk::Extent2D &extent
) : AttachmentGroupBase { extent } { }

auto vku::AttachmentGroup::addColorAttachment(
    const vk::raii::Device &device,
    const Image &image,
    vk::Format viewFormat
) -> const Attachment & {
    return addColorAttachment(device, image, vk::ImageViewCreateInfo {
        {},
        image,
        vk::ImageViewType::e2D,
        viewFormat == vk::Format::eUndefined ? image.format : viewFormat,
        {},
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
    });
}

auto vku::AttachmentGroup::addColorAttachment(
    const vk::raii::Device &device,
    const Image &image,
    const vk::ImageViewCreateInfo &viewCreateInfo
) -> const Attachment & {
    return colorAttachments.emplace_back(image, vk::raii::ImageView { device, viewCreateInfo });
}

auto vku::AttachmentGroup::createColorImage(
    vma::Allocator allocator,
    vk::Format format,
    vk::ImageUsageFlags usage,
    const vma::AllocationCreateInfo &allocationCreateInfo
) const -> AllocatedImage {
    return createAttachmentImage(
        allocator,
        format,
        vk::SampleCountFlagBits::e1,
        usage,
        allocationCreateInfo);
}

auto vku::AttachmentGroup::setDepthStencilAttachment(
    const vk::raii::Device &device,
    const Image &image,
    vk::Format viewFormat
) -> const Attachment & {
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

auto vku::AttachmentGroup::setDepthStencilAttachment(
    const vk::raii::Device &device,
    const Image &image,
    const vk::ImageViewCreateInfo &viewCreateInfo
) -> const Attachment & {
    return depthStencilAttachment.emplace(image, vk::raii::ImageView { device, viewCreateInfo });
}

auto vku::AttachmentGroup::createDepthStencilImage(
    vma::Allocator allocator,
    vk::Format format,
    vk::ImageUsageFlags usage,
    const vma::AllocationCreateInfo &allocationCreateInfo
) const -> AllocatedImage {
    return createAttachmentImage(
        allocator,
        format,
        vk::SampleCountFlagBits::e1,
        usage,
        allocationCreateInfo);
}

auto vku::AttachmentGroup::getRenderingInfo(
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
        ranges::views::zip_transform([](const Attachment &attachment, const ColorAttachmentInfo &info) {
            return vk::RenderingAttachmentInfo {
                *attachment.view, vk::ImageLayout::eColorAttachmentOptimal,
                {}, {}, {},
                info.loadOp, info.storeOp, info.clearValue,
            };
        }, colorAttachments, colorAttachmentInfos) | std::ranges::to<std::vector>(),
    };
}

auto vku::AttachmentGroup::getRenderingInfo(
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
        ranges::views::zip_transform([](const Attachment &attachment, const ColorAttachmentInfo &info) {
            return vk::RenderingAttachmentInfo {
                *attachment.view, vk::ImageLayout::eColorAttachmentOptimal,
                {}, {}, {},
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