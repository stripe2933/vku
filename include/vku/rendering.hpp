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
        auto setViewport(vk::CommandBuffer commandBuffer ,bool negativeViewport = false) const -> void;
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

    struct MsaaAttachmentGroup : AttachmentGroupBase {
        struct MsaaAttachment : Attachment {
            Image resolveImage;
            vk::raii::ImageView resolveView;
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

        auto addColorAttachment(
            const vk::raii::Device &device,
            const Image &image,
            const Image &resolveImage,
            vk::Format format = {},
            const vk::ImageSubresourceRange &subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
            const vk::ImageSubresourceRange &resolveSubresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        ) -> const MsaaAttachment&;

        [[nodiscard]] auto createColorImage(
            vma::Allocator allocator,
            vk::Format format,
            vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransientAttachment,
            const vma::AllocationCreateInfo &allocationCreateInfo = { {}, vma::MemoryUsage::eAutoPreferDevice, {}, vk::MemoryPropertyFlagBits::eLazilyAllocated }
        ) const -> AllocatedImage;

        [[nodiscard]] auto createResolveImage(
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
            vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransientAttachment,
            const vma::AllocationCreateInfo &allocationCreateInfo = { {}, vma::MemoryUsage::eAutoPreferDevice, {}, vk::MemoryPropertyFlagBits::eLazilyAllocated }
        ) const -> AllocatedImage;

        auto getRenderingInfo(
            std::span<const std::tuple<vk::AttachmentLoadOp, vk::AttachmentStoreOp, vk::ClearColorValue>> colorAttachmentInfos = {},
            const std::optional<std::tuple<vk::AttachmentLoadOp, vk::AttachmentStoreOp, vk::ClearDepthStencilValue>> &depthStencilAttachmentInfo = {}
        ) const -> RefHolder<vk::RenderingInfo, std::vector<vk::RenderingAttachmentInfo>, std::optional<vk::RenderingAttachmentInfo>> override;
    };
}