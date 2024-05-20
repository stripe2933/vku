#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "images.hpp"
#include "RefHolder.hpp"

#include "details/macros.hpp"

namespace vku {
    struct Attachment {
        Image image;
        vk::raii::ImageView view;
    };

    class AttachmentGroupBase {
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
            return *storedImage.emplace_back(std::make_unique<std::remove_cvref_t<decltype(image)>>(VKU_FWD(image)));
        }
        auto setViewport(vk::CommandBuffer commandBuffer, bool negativeViewport = false) const -> void;
        auto setScissor(vk::CommandBuffer commandBuffer) const -> void;

        [[nodiscard]] virtual auto getRenderingInfo(
            std::span<const std::tuple<vk::AttachmentLoadOp, vk::AttachmentStoreOp, vk::ClearColorValue>> colorAttachmentInfos = {},
            const std::optional<std::tuple<vk::AttachmentLoadOp, vk::AttachmentStoreOp, vk::ClearDepthStencilValue>> &depthStencilAttachmentInfo = {}
        ) const -> RefHolder<vk::RenderingInfo, std::vector<vk::RenderingAttachmentInfo>, std::optional<vk::RenderingAttachmentInfo>> = 0;

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