/** @file rendering/AttachmentGroup.cppm
 */

module;

#include <cassert>

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:rendering.AttachmentGroup;

import std;
import vku.details;
export import :rendering.Attachment;
export import :rendering.AttachmentGroupBase;
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

        std::vector<std::variant<Attachment, SwapchainAttachment>> colorAttachments;
        std::optional<Attachment> depthStencilAttachment;

        explicit AttachmentGroup(const VULKAN_HPP_NAMESPACE::Extent2D &extent);
        AttachmentGroup(const AttachmentGroup&) = delete;
        AttachmentGroup(AttachmentGroup&&) noexcept = default;
        auto operator=(const AttachmentGroup&) -> AttachmentGroup& = delete;
        auto operator=(AttachmentGroup&&) noexcept -> AttachmentGroup& = default;
        ~AttachmentGroup() override = default;

        auto addColorAttachment(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
            const Image &image,
            VULKAN_HPP_NAMESPACE::Format viewFormat = {}
        ) -> const Attachment&;
        auto addColorAttachment(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
            const Image &image,
            const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &viewCreateInfo
        ) -> const Attachment&;
        auto addSwapchainAttachment(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
            std::span<const VULKAN_HPP_NAMESPACE::Image> swapchainImages,
            VULKAN_HPP_NAMESPACE::Format viewFormat
        ) -> const SwapchainAttachment&;
        auto setDepthStencilAttachment(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
            const Image &image,
            VULKAN_HPP_NAMESPACE::Format viewFormat = {}
        ) -> const Attachment&;
        auto setDepthStencilAttachment(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
            const Image &image,
            const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &viewCreateInfo
        ) -> const Attachment&;

        /**
         * Get the const reference of color attachment at the specified index.
         * @param index The index of the color attachment.
         * @return The const reference of the color attachment.
         * @note This function is designed to make it easier for users to access elements from <tt>attachmentGroup.colorAttachments[i]</tt>
         * without needing to use the more cumbersome <tt>get<vku::Attachment>(...)</tt> call.
         * @note This function is considered "unsafe" because if the element at the specified index isn't a <tt>vku::Attachment</tt>,
         * it can lead to undefined behavior.
         */
        [[nodiscard]] auto getColorAttachment(std::size_t index) const noexcept -> const Attachment& {
            return *get_if<Attachment>(&colorAttachments[index]);
        }

        /**
         * Get the const reference of swapchain attachment at the specified index.
         * @param index The index of the swapchain attachment.
         * @return The const reference of the swapchain attachment.
         * @note This function is designed to make it easier for users to access elements from <tt>attachmentGroup.colorAttachments[i]</tt>
         * without needing to use the more cumbersome <tt>get<vku::SwapchainAttachment>(...)</tt> call.
         * @note This function is considered "unsafe" because if the element at the specified index isn't a <tt>vku::SwapchainAttachment</tt>,
         * it can lead to undefined behavior.
         */
        [[nodiscard]] auto getSwapchainAttachment(std::size_t index) const noexcept -> const SwapchainAttachment& {
            return *get_if<SwapchainAttachment>(&colorAttachments[index]);
        }

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
            VULKAN_HPP_NAMESPACE::ArrayProxy<const ColorAttachmentInfo> colorAttachmentInfos
        ) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>>;

        [[nodiscard]] auto getRenderingInfo(
            VULKAN_HPP_NAMESPACE::ArrayProxy<const ColorAttachmentInfo> colorAttachmentInfos,
            std::uint32_t swapchainImageIndex
        ) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>>;

        [[nodiscard]] auto getRenderingInfo(
            VULKAN_HPP_NAMESPACE::ArrayProxy<const ColorAttachmentInfo> colorAttachmentInfos,
            const DepthStencilAttachmentInfo &depthStencilAttachmentInfo
        ) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>>;

        [[nodiscard]] auto getRenderingInfo(
            VULKAN_HPP_NAMESPACE::ArrayProxy<const ColorAttachmentInfo> colorAttachmentInfos,
            const DepthStencilAttachmentInfo &depthStencilAttachmentInfo,
            std::uint32_t swapchainImageIndex
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
    return *get_if<Attachment>(&colorAttachments.emplace_back(
        std::in_place_type<Attachment>,
        image,
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView { device, viewCreateInfo }));
}

auto vku::AttachmentGroup::addSwapchainAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    std::span<const VULKAN_HPP_NAMESPACE::Image> swapchainImages,
    VULKAN_HPP_NAMESPACE::Format viewFormat
) -> const SwapchainAttachment& {
    std::vector<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView> views;
    views.reserve(swapchainImages.size());
    for (VULKAN_HPP_NAMESPACE::Image swapchainImage : swapchainImages) {
        views.emplace_back(device, VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
            {},
            swapchainImage,
            VULKAN_HPP_NAMESPACE::ImageViewType::e2D,
            viewFormat,
            {},
            { VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
        });
    }

    return *get_if<SwapchainAttachment>(&colorAttachments.emplace_back(
        std::in_place_type<SwapchainAttachment>,
        std::move(views)));
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
    VULKAN_HPP_NAMESPACE::ArrayProxy<const ColorAttachmentInfo> colorAttachmentInfos
) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(!depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");

    std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> renderingAttachmentInfos;
    renderingAttachmentInfos.reserve(colorAttachmentInfos.size());
    for (const auto &[attachment, info] : std::views::zip(colorAttachments, colorAttachmentInfos)) {
        auto *const pAttachment = get_if<Attachment>(&attachment);
        assert(pAttachment && "A SwapchainAttachment is in the attachment group.");
        renderingAttachmentInfos.push_back({
            *pAttachment->view, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
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
    VULKAN_HPP_NAMESPACE::ArrayProxy<const ColorAttachmentInfo> colorAttachmentInfos,
    std::uint32_t swapchainImageIndex
) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(!depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");

    std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> renderingAttachmentInfos;
    renderingAttachmentInfos.reserve(colorAttachmentInfos.size());

    // SwapchainAttachment can only appear once in the attachment group. Therefore, if a SwapchainAttachment already
    // push_backed into renderingAttachmentInfos, explicit std::visit call for the rest variants is not necessary.
    bool swapchainAttachmentPushed = false;
    for (const auto &[attachment, info] : std::views::zip(colorAttachments, colorAttachmentInfos)) {
        const VULKAN_HPP_NAMESPACE::ImageView imageView = [&]() {
            if (swapchainAttachmentPushed) {
                auto *const pAttachment = get_if<Attachment>(&attachment);
                assert(pAttachment && "More than one SwapchainAttachment in the attachment group.");
                return *pAttachment->view;
            }

            return visit(details::multilambda {
                [](const Attachment &attachment) {
                    return *attachment.view;
                },
                [&](const SwapchainAttachment &swapchainAttachment) {
                    swapchainAttachmentPushed = true;
                    return *swapchainAttachment.views[swapchainImageIndex];
                },
            }, attachment);
        }();

        renderingAttachmentInfos.push_back({
            imageView, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
            {}, {}, {},
            info.loadOp, info.storeOp, info.clearValue,
        });
    }
    assert(swapchainAttachmentPushed && "swapchainImageIndex set but there is no swapchain attachment in the attachment group.");

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
    VULKAN_HPP_NAMESPACE::ArrayProxy<const ColorAttachmentInfo> colorAttachmentInfos,
    const DepthStencilAttachmentInfo &depthStencilAttachmentInfo
) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");

    std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> renderingAttachmentInfos;
    renderingAttachmentInfos.reserve(colorAttachmentInfos.size() + 1);
    for (const auto &[attachment, info] : std::views::zip(colorAttachments, colorAttachmentInfos)) {
        auto *const pAttachment = get_if<Attachment>(&attachment);
        assert(pAttachment && "A SwapchainAttachment is in the attachment group.");
        renderingAttachmentInfos.push_back({
            *pAttachment->view, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
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

auto vku::AttachmentGroup::getRenderingInfo(
    VULKAN_HPP_NAMESPACE::ArrayProxy<const ColorAttachmentInfo> colorAttachmentInfos,
    const DepthStencilAttachmentInfo &depthStencilAttachmentInfo,
    std::uint32_t swapchainImageIndex
) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");

    std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> renderingAttachmentInfos;
    renderingAttachmentInfos.reserve(colorAttachmentInfos.size() + 1);

    // SwapchainAttachment can only appear once in the attachment group. Therefore, if a SwapchainAttachment already
    // push_backed into renderingAttachmentInfos, explicit std::visit call for the rest variants is not necessary.
    bool swapchainAttachmentPushed = false;
    for (const auto &[attachment, info] : std::views::zip(colorAttachments, colorAttachmentInfos)) {
        const VULKAN_HPP_NAMESPACE::ImageView imageView = [&]() {
            if (swapchainAttachmentPushed) {
                auto *const pAttachment = get_if<Attachment>(&attachment);
                assert(pAttachment && "More than one SwapchainAttachment in the attachment group.");
                return *pAttachment->view;
            }

            return visit(details::multilambda {
                [](const Attachment &attachment) {
                    return *attachment.view;
                },
                [&](const SwapchainAttachment &swapchainAttachment) {
                    swapchainAttachmentPushed = true;
                    return *swapchainAttachment.views[swapchainImageIndex];
                },
            }, attachment);
        }();

        renderingAttachmentInfos.push_back({
            imageView, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
            {}, {}, {},
            info.loadOp, info.storeOp, info.clearValue,
        });
    }
    assert(swapchainAttachmentPushed && "swapchainImageIndex set but there is no swapchain attachment in the attachment group.");
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