#pragma once

#include "rendering.hpp"

namespace vku {
    struct AttachmentGroup : AttachmentGroupBase {
        std::vector<Attachment> colorAttachments;
        std::optional<Attachment> depthStencilAttachment;

        explicit AttachmentGroup(const vk::Extent2D &extent);
        AttachmentGroup(const AttachmentGroup&) = delete;
        AttachmentGroup(AttachmentGroup&&) noexcept = default;
        auto operator=(const AttachmentGroup&) -> AttachmentGroup& = delete;
        auto operator=(AttachmentGroup&&) noexcept -> AttachmentGroup& = default;
        ~AttachmentGroup() override = default;

        auto addColorAttachment(
            const vk::raii::Device &device,
            const Image &image,
            vk::Format format = {},
            const vk::ImageSubresourceRange &subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        ) -> const Attachment&;

        [[nodiscard]] auto createColorImage(
            vma::Allocator allocator,
            vk::Format format,
            vk::ImageUsageFlags usage = {},
            const vma::AllocationCreateInfo &allocationCreateInfo = { {}, vma::MemoryUsage::eAutoPreferDevice }
        ) const -> AllocatedImage;

        auto setDepthAttachment(
            const vk::raii::Device &device,
            const Image &image,
            vk::Format format = {},
            const vk::ImageSubresourceRange &subresourceRange = { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 }
        ) -> const Attachment&;

        auto setStencilAttachment(
            const vk::raii::Device &device,
            const Image &image,
            vk::Format format = {},
            const vk::ImageSubresourceRange &subresourceRange = { vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1 }
        ) -> const Attachment&;

        auto setDepthStencilAttachment(
            const vk::raii::Device &device,
            const Image &image,
            vk::Format format = {},
            const vk::ImageSubresourceRange &subresourceRange = { vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1 }
        ) -> const Attachment&;

        [[nodiscard]] auto createDepthStencilImage(
            vma::Allocator allocator,
            vk::Format format,
            vk::ImageUsageFlags usage = {},
            const vma::AllocationCreateInfo &allocationCreateInfo = { {}, vma::MemoryUsage::eAutoPreferDevice }
        ) const -> AllocatedImage;

        auto getRenderingInfo(
            std::span<const std::tuple<vk::AttachmentLoadOp, vk::AttachmentStoreOp, vk::ClearColorValue>> colorAttachmentInfos = {},
            const std::optional<std::tuple<vk::AttachmentLoadOp, vk::AttachmentStoreOp, vk::ClearDepthStencilValue>> &depthStencilAttachmentInfo = {}
        ) const -> RefHolder<vk::RenderingInfo, std::vector<vk::RenderingAttachmentInfo>, std::optional<vk::RenderingAttachmentInfo>> override;
    };
}