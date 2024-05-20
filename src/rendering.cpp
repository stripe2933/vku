#include <vku/rendering.hpp>

vku::AttachmentGroupBase::AttachmentGroupBase(
    const vk::Extent2D &extent
) : extent { extent } { }

auto vku::AttachmentGroupBase::storeImage(
    std::unique_ptr<AllocatedImage> image
) -> const AllocatedImage& {
    return *storedImage.emplace_back(std::move(image));
}

auto vku::AttachmentGroupBase::setViewport(
    vk::CommandBuffer commandBuffer,
    bool negativeViewport
) const -> void {
    if (negativeViewport) {
        commandBuffer.setViewport(0, vk::Viewport {
            0, static_cast<float>(extent.height),
            static_cast<float>(extent.width), -static_cast<float>(extent.height),
            0.f, 1.f,
        });
    }
    else {
        commandBuffer.setViewport(0, vk::Viewport {
            0, 0,
            static_cast<float>(extent.width), static_cast<float>(extent.height),
            0.f, 1.f,
        });
    }
}

auto vku::AttachmentGroupBase::setScissor(
    vk::CommandBuffer commandBuffer
) const -> void {
    commandBuffer.setScissor(0, vk::Rect2D {
        { 0, 0 },
        extent,
    });
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