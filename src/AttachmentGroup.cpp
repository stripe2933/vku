#include <vku/AttachmentGroup.hpp>

#include <vku/details/ranges.hpp>

vku::AttachmentGroup::AttachmentGroup(
    const vk::Extent2D &extent
) : AttachmentGroupBase { extent } { }

auto vku::AttachmentGroup::addColorAttachment(
    const vk::raii::Device &device,
    const Image &image,
    vk::Format format,
    const vk::ImageSubresourceRange &subresourceRange
) -> const Attachment & {
    return colorAttachments.emplace_back(
        image,
        vk::raii::ImageView { device, vk::ImageViewCreateInfo {
            {},
            image,
            vk::ImageViewType::e2D,
            format == vk::Format::eUndefined ? image.format : format,
            {},
            subresourceRange,
        } });
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
        usage | vk::ImageUsageFlagBits::eColorAttachment,
        allocationCreateInfo);
}

auto vku::AttachmentGroup::setDepthAttachment(
    const vk::raii::Device &device,
    const Image &image,
    vk::Format format,
    const vk::ImageSubresourceRange &subresourceRange
) -> const Attachment& {
    return depthStencilAttachment.emplace(
        image,
        vk::raii::ImageView { device, vk::ImageViewCreateInfo {
            {},
            image,
            vk::ImageViewType::e2D,
            format == vk::Format::eUndefined ? image.format : format,
            {},
            subresourceRange,
        } });
}

auto vku::AttachmentGroup::setStencilAttachment(
    const vk::raii::Device &device,
    const Image &image,
    vk::Format format,
    const vk::ImageSubresourceRange &subresourceRange
) -> const Attachment& {
    return depthStencilAttachment.emplace(
        image,
        vk::raii::ImageView { device, vk::ImageViewCreateInfo {
            {},
            image,
            vk::ImageViewType::e2D,
            format == vk::Format::eUndefined ? image.format : format,
            {},
            subresourceRange,
        } });
}

auto vku::AttachmentGroup::setDepthStencilAttachment(
    const vk::raii::Device &device, const Image &image,
    vk::Format format,
    const vk::ImageSubresourceRange &subresourceRange
) -> const Attachment & {
    return depthStencilAttachment.emplace(
        image,
        vk::raii::ImageView { device, vk::ImageViewCreateInfo {
            {},
            image,
            vk::ImageViewType::e2D,
            format == vk::Format::eUndefined ? image.format : format,
            {},
            subresourceRange,
        } });
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
        usage | vk::ImageUsageFlagBits::eDepthStencilAttachment,
        allocationCreateInfo);
}

auto vku::AttachmentGroup::getRenderingInfo(
    std::span<const std::tuple<vk::AttachmentLoadOp, vk::AttachmentStoreOp, vk::ClearColorValue>> colorAttachmentInfos,
    const std::optional<std::tuple<vk::AttachmentLoadOp, vk::AttachmentStoreOp, vk::ClearDepthStencilValue>> &depthStencilAttachmentInfo
) const -> RefHolder<vk::RenderingInfo, std::vector<vk::RenderingAttachmentInfo>, std::optional<vk::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(depthStencilAttachment.has_value() == depthStencilAttachmentInfo.has_value() && "Depth-stencil attachment info mismatch");
    return {
        [this](std::span<const vk::RenderingAttachmentInfo> colorAttachmentInfos, const std::optional<vk::RenderingAttachmentInfo> &depthStencilAttachmentInfo) {
            return vk::RenderingInfo {
                {},
                { { 0, 0 }, extent },
                1,
                {},
                colorAttachmentInfos,
                depthStencilAttachmentInfo ? &*depthStencilAttachmentInfo : nullptr,
            };
        },
        details::ranges::views::zip_transform([](const Attachment &attachment, const std::tuple<vk::AttachmentLoadOp, vk::AttachmentStoreOp, vk::ClearColorValue> &info) {
            return vk::RenderingAttachmentInfo {
                *attachment.view, vk::ImageLayout::eColorAttachmentOptimal,
                {}, {}, {},
                std::get<0>(info), std::get<1>(info), std::get<2>(info),
            };
        }, colorAttachments, colorAttachmentInfos) | std::ranges::to<std::vector>(),
        depthStencilAttachment.transform([info = *depthStencilAttachmentInfo](const Attachment &attachment) {
            return vk::RenderingAttachmentInfo {
                *attachment.view, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                {}, {}, {},
                std::get<0>(info), std::get<1>(info), std::get<2>(info),
            };
        })
    };
}