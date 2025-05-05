/** @file rendering/MultisampleAttachmentGroup.cppm
 */

module;

#include <cassert>

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:rendering.MultisampleAttachmentGroup;

import std;
import vku.details;
export import :rendering.Attachment;
export import :rendering.AttachmentGroupBase;
export import :rendering.MultisampleAttachment;
import :utils.RefHolder;

// #define VMA_HPP_NAMESPACE to vma, if not defined.
#ifndef VMA_HPP_NAMESPACE
#define VMA_HPP_NAMESPACE vma
#endif

namespace vku {
    export struct MultisampleAttachmentGroup : AttachmentGroupBase {
        struct ColorAttachmentInfo {
            VULKAN_HPP_NAMESPACE::AttachmentLoadOp loadOp;
            VULKAN_HPP_NAMESPACE::AttachmentStoreOp storeOp;
            VULKAN_HPP_NAMESPACE::ClearColorValue clearValue;
            VULKAN_HPP_NAMESPACE::ResolveModeFlagBits resolveMode = VULKAN_HPP_NAMESPACE::ResolveModeFlagBits::eAverage;
        };

        struct DepthStencilAttachmentInfo {
            VULKAN_HPP_NAMESPACE::AttachmentLoadOp loadOp;
            VULKAN_HPP_NAMESPACE::AttachmentStoreOp storeOp;
            VULKAN_HPP_NAMESPACE::ClearDepthStencilValue clearValue;
        };

        VULKAN_HPP_NAMESPACE::SampleCountFlagBits sampleCount;
        std::vector<std::variant<MultisampleAttachment, SwapchainMultisampleAttachment>> colorAttachments;
        std::optional<Attachment> depthStencilAttachment;

        explicit MultisampleAttachmentGroup(const VULKAN_HPP_NAMESPACE::Extent2D &extent, VULKAN_HPP_NAMESPACE::SampleCountFlagBits sampleCount);
        MultisampleAttachmentGroup(const MultisampleAttachmentGroup&) = delete;
        MultisampleAttachmentGroup(MultisampleAttachmentGroup&&) noexcept = default;
        auto operator=(const MultisampleAttachmentGroup&) -> MultisampleAttachmentGroup& = delete;
        auto operator=(MultisampleAttachmentGroup&&) noexcept -> MultisampleAttachmentGroup& = default;
        ~MultisampleAttachmentGroup() override = default;

        auto addColorAttachment(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
            const Image &multisampleImage,
            const Image &image,
            VULKAN_HPP_NAMESPACE::Format viewFormat = {}
        ) -> const MultisampleAttachment&;
        auto addColorAttachment(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
            const Image &multisampleImage,
            const Image &image,
            const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &multisampleViewCreateInfo,
            const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &viewCreateInfo
        ) -> const MultisampleAttachment&;
        auto addSwapchainAttachment(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
            const Image &multisampleImage,
            std::span<const VULKAN_HPP_NAMESPACE::Image> swapchainImages,
            VULKAN_HPP_NAMESPACE::Format viewFormat = {}
        ) -> const SwapchainMultisampleAttachment&;
        auto addSwapchainAttachment(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
            const Image &multisampleImage,
            std::span<const VULKAN_HPP_NAMESPACE::Image> swapchainImages,
            const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &multisampleViewCreateInfo
        ) -> const SwapchainMultisampleAttachment&;
        auto setDepthStencilAttachment(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
            const Image &multisampleImage,
            VULKAN_HPP_NAMESPACE::Format viewFormat = {}
        ) -> const Attachment&;
        auto setDepthStencilAttachment(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
            const Image &multisampleImage,
            const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &multisampleViewCreateInfo
            ) -> const Attachment&;

        /**
         * Get the const reference of color attachment at the specified index.
         * @param index The index of the color attachment.
         * @return The const reference of the color attachment.
         * @note This function is designed to make it easier for users to access elements from <tt>attachmentGroup.colorAttachments[i]</tt>
         * without needing to use the more cumbersome <tt>get<vku::MultisampleAttachment>(...)</tt> call.
         * @note This function is considered "unsafe" because if the element at the specified index isn't a <tt>vku::MultisampleAttachment</tt>,
         * it can lead to undefined behavior.
         */
        [[nodiscard]] auto getColorAttachment(std::size_t index) const noexcept -> const MultisampleAttachment& {
            return *get_if<MultisampleAttachment>(&colorAttachments[index]);
        }

        /**
         * Get the const reference of swapchain attachment at the specified index.
         * @param index The index of the swapchain attachment.
         * @return The const reference of the swapchain attachment.
         * @note This function is designed to make it easier for users to access elements from <tt>attachmentGroup.colorAttachments[i]</tt>
         * without needing to use the more cumbersome <tt>get<vku::SwapchainMultisampleAttachment>(...)</tt> call.
         * @note This function is considered "unsafe" because if the element at the specified index isn't a <tt>vku::SwapchainMultisampleAttachment</tt>,
         * it can lead to undefined behavior.
         */
        [[nodiscard]] auto getSwapchainAttachment(std::size_t index) const noexcept -> const SwapchainMultisampleAttachment& {
            return *get_if<SwapchainMultisampleAttachment>(&colorAttachments[index]);
        }

        [[nodiscard]] auto createColorImage(
            VMA_HPP_NAMESPACE::Allocator allocator,
            VULKAN_HPP_NAMESPACE::Format format,
            VULKAN_HPP_NAMESPACE::ImageUsageFlags usage = VULKAN_HPP_NAMESPACE::ImageUsageFlagBits::eColorAttachment | VULKAN_HPP_NAMESPACE::ImageUsageFlagBits::eTransientAttachment,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = { {}, VMA_HPP_NAMESPACE::MemoryUsage::eAutoPreferDevice, {}, VULKAN_HPP_NAMESPACE::MemoryPropertyFlagBits::eLazilyAllocated }
        ) const -> AllocatedImage;

        [[nodiscard]] auto createResolveImage(
            VMA_HPP_NAMESPACE::Allocator allocator,
            VULKAN_HPP_NAMESPACE::Format viewFormat,
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

vku::MultisampleAttachmentGroup::MultisampleAttachmentGroup(
    const VULKAN_HPP_NAMESPACE::Extent2D &extent,
    VULKAN_HPP_NAMESPACE::SampleCountFlagBits sampleCount
) : AttachmentGroupBase { extent },
    sampleCount { sampleCount } { }

auto vku::MultisampleAttachmentGroup::addColorAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &multisampleImage,
    const Image &image,
    VULKAN_HPP_NAMESPACE::Format viewFormat
) -> const MultisampleAttachment& {
    return addColorAttachment(
        device,
        multisampleImage,
        image,
        VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
            {},
            multisampleImage,
            VULKAN_HPP_NAMESPACE::ImageViewType::e2D,
            viewFormat == VULKAN_HPP_NAMESPACE::Format::eUndefined ? multisampleImage.format : viewFormat,
            {},
            { VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
        },
        VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
            {},
            image,
            VULKAN_HPP_NAMESPACE::ImageViewType::e2D,
            viewFormat == VULKAN_HPP_NAMESPACE::Format::eUndefined ? image.format : viewFormat,
            {},
            { VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
        });
}

auto vku::MultisampleAttachmentGroup::addColorAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &multisampleImage,
    const Image &image,
    const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &multisampleViewCreateInfo,
    const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &viewCreateInfo
) -> const MultisampleAttachment & {
    return *get_if<MultisampleAttachment>(&colorAttachments.emplace_back(
        std::in_place_type<MultisampleAttachment>,
        multisampleImage,
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView { device, multisampleViewCreateInfo },
        image,
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView { device, viewCreateInfo }));
}

auto vku::MultisampleAttachmentGroup::addSwapchainAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &multisampleImage,
    std::span<const VULKAN_HPP_NAMESPACE::Image> swapchainImages,
    VULKAN_HPP_NAMESPACE::Format viewFormat
) -> const SwapchainMultisampleAttachment& {
    return addSwapchainAttachment(
        device,
        multisampleImage,
        swapchainImages,
        VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
            {},
            multisampleImage,
            VULKAN_HPP_NAMESPACE::ImageViewType::e2D,
            viewFormat == VULKAN_HPP_NAMESPACE::Format::eUndefined ? multisampleImage.format : viewFormat,
            {},
            { VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
        });
}

auto vku::MultisampleAttachmentGroup::addSwapchainAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &multisampleImage,
    std::span<const VULKAN_HPP_NAMESPACE::Image> swapchainImages,
    const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &multisampleViewCreateInfo
) -> const SwapchainMultisampleAttachment& {
    std::vector<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView> resolveViews;
    resolveViews.reserve(swapchainImages.size());
    for (VULKAN_HPP_NAMESPACE::Image swapchainImage : swapchainImages) {
        resolveViews.emplace_back(device, VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
            {},
            swapchainImage,
            VULKAN_HPP_NAMESPACE::ImageViewType::e2D,
            multisampleViewCreateInfo.format,
            {},
            { VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
        });
    }

    return *get_if<SwapchainMultisampleAttachment>(&colorAttachments.emplace_back(
        std::in_place_type<SwapchainMultisampleAttachment>,
        multisampleImage,
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView { device, multisampleViewCreateInfo },
        std::move(resolveViews)));
}

auto vku::MultisampleAttachmentGroup::setDepthStencilAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &multisampleImage,
    VULKAN_HPP_NAMESPACE::Format viewFormat
) -> const Attachment& {
    if (viewFormat == VULKAN_HPP_NAMESPACE::Format::eUndefined) {
        viewFormat = multisampleImage.format;
    }
    return setDepthStencilAttachment(device, multisampleImage, VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
        {},
        multisampleImage,
        VULKAN_HPP_NAMESPACE::ImageViewType::e2D,
        viewFormat,
        {},
        { Image::inferAspectFlags(viewFormat), 0, 1, 0, 1 },
    });
}

auto vku::MultisampleAttachmentGroup::setDepthStencilAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &multisampleImage,
    const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &multisampleViewCreateInfo
) -> const Attachment & {
    return depthStencilAttachment.emplace(multisampleImage, VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView { device, multisampleViewCreateInfo });
}

auto vku::MultisampleAttachmentGroup::createColorImage(
    VMA_HPP_NAMESPACE::Allocator allocator,
    VULKAN_HPP_NAMESPACE::Format format,
    VULKAN_HPP_NAMESPACE::ImageUsageFlags usage,
    const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
) const -> AllocatedImage {
    return createAttachmentImage(allocator, format, sampleCount, usage, allocationCreateInfo);
}

auto vku::MultisampleAttachmentGroup::createResolveImage(
    VMA_HPP_NAMESPACE::Allocator allocator,
    VULKAN_HPP_NAMESPACE::Format viewFormat,
    VULKAN_HPP_NAMESPACE::ImageUsageFlags usage,
    const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
) const -> AllocatedImage {
    return createAttachmentImage(allocator, viewFormat, VULKAN_HPP_NAMESPACE::SampleCountFlagBits::e1, usage, allocationCreateInfo);
}

auto vku::MultisampleAttachmentGroup::createDepthStencilImage(
    VMA_HPP_NAMESPACE::Allocator allocator,
    VULKAN_HPP_NAMESPACE::Format format,
    VULKAN_HPP_NAMESPACE::ImageUsageFlags usage,
    const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
) const -> AllocatedImage {
    return createAttachmentImage(
        allocator,
        format,
        sampleCount,
        usage,
        allocationCreateInfo);
}

auto vku::MultisampleAttachmentGroup::getRenderingInfo(
    VULKAN_HPP_NAMESPACE::ArrayProxy<const ColorAttachmentInfo> colorAttachmentInfos
) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(!depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");

    std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> renderingAttachmentInfos;
    renderingAttachmentInfos.reserve(colorAttachmentInfos.size());
    for (const auto &[attachment, info] : std::views::zip(colorAttachments, colorAttachmentInfos)) {
        auto *const pMultisampleAttachment = get_if<MultisampleAttachment>(&attachment);
        assert(pMultisampleAttachment && "More than one SwapchainMultisampleAttachment in the attachment group.");
        renderingAttachmentInfos.push_back({
            *pMultisampleAttachment->multisampleView, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
            info.resolveMode, *pMultisampleAttachment->view, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
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

auto vku::MultisampleAttachmentGroup::getRenderingInfo(
    VULKAN_HPP_NAMESPACE::ArrayProxy<const ColorAttachmentInfo> colorAttachmentInfos,
    std::uint32_t swapchainImageIndex
) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(!depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");

    std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> renderingAttachmentInfos;
    renderingAttachmentInfos.reserve(colorAttachmentInfos.size());

    // SwapchainMultisampleAttachment can only appear once in the attachment group. Therefore, if a SwapchainMultisampleAttachment
    // already push_backed into renderingAttachmentInfos, explicit std::visit call for the rest variants is not necessary.
    bool swapchainAttachmentPushed = false;
    for (const auto &[attachment, info] : std::views::zip(colorAttachments, colorAttachmentInfos)) {
        const auto [view, resolveView] = [&]() {
            if (swapchainAttachmentPushed) {
                auto *pMultisampleAttachment = get_if<MultisampleAttachment>(&attachment);
                assert(pMultisampleAttachment && "More than one SwapchainMultisampleAttachment in the attachment group.");
                return std::pair { *pMultisampleAttachment->multisampleView, *pMultisampleAttachment->view };
            }

            return visit(details::multilambda {
                [](const MultisampleAttachment &multisampleAttachment) {
                    return std::pair { *multisampleAttachment.multisampleView, *multisampleAttachment.view };
                },
                [&](const SwapchainMultisampleAttachment &swapchainMultisampleAttachment) {
                    swapchainAttachmentPushed = true;
                    return std::pair { *swapchainMultisampleAttachment.multisampleView, *swapchainMultisampleAttachment.views[swapchainImageIndex] };
                },
            }, attachment);
        }();
        renderingAttachmentInfos.push_back({
            view, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
            info.resolveMode, resolveView, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
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

auto vku::MultisampleAttachmentGroup::getRenderingInfo(
    VULKAN_HPP_NAMESPACE::ArrayProxy<const ColorAttachmentInfo> colorAttachmentInfos,
    const DepthStencilAttachmentInfo &depthStencilAttachmentInfo
) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");

    std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> renderingAttachmentInfos;
    renderingAttachmentInfos.reserve(colorAttachmentInfos.size() + 1);
    for (const auto &[attachment, info] : std::views::zip(colorAttachments, colorAttachmentInfos)) {
        auto *const pMultisampleAttachment = get_if<MultisampleAttachment>(&attachment);
        assert(pMultisampleAttachment && "More than one SwapchainMultisampleAttachment in the attachment group.");
        renderingAttachmentInfos.push_back({
            *pMultisampleAttachment->multisampleView, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
            info.resolveMode, *pMultisampleAttachment->view, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
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

auto vku::MultisampleAttachmentGroup::getRenderingInfo(
    VULKAN_HPP_NAMESPACE::ArrayProxy<const ColorAttachmentInfo> colorAttachmentInfos,
    const DepthStencilAttachmentInfo &depthStencilAttachmentInfo,
    std::uint32_t swapchainImageIndex
) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");

    std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> renderingAttachmentInfos;
    renderingAttachmentInfos.reserve(colorAttachmentInfos.size() + 1);

    // SwapchainMultisampleAttachment can only appear once in the attachment group. Therefore, if a SwapchainMultisampleAttachment
    // already push_backed into renderingAttachmentInfos, explicit std::visit call for the rest variants is not necessary.
    bool swapchainAttachmentPushed = false;
    for (const auto &[attachment, info] : std::views::zip(colorAttachments, colorAttachmentInfos)) {
        const auto [view, resolveView] = [&]() {
            if (swapchainAttachmentPushed) {
                auto *pMultisampleAttachment = get_if<MultisampleAttachment>(&attachment);
                assert(pMultisampleAttachment && "More than one SwapchainMultisampleAttachment in the attachment group.");
                return std::pair { *pMultisampleAttachment->multisampleView, *pMultisampleAttachment->view };
            }

            return visit(details::multilambda {
                [](const MultisampleAttachment &multisampleAttachment) {
                    return std::pair { *multisampleAttachment.multisampleView, *multisampleAttachment.view };
                },
                [&](const SwapchainMultisampleAttachment &swapchainMultisampleAttachment) {
                    swapchainAttachmentPushed = true;
                    return std::pair { *swapchainMultisampleAttachment.multisampleView, *swapchainMultisampleAttachment.views[swapchainImageIndex] };
                },
            }, attachment);
        }();
        renderingAttachmentInfos.push_back({
            view, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
            info.resolveMode, resolveView, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
            info.loadOp, info.storeOp, info.clearValue,
        });
    }
    renderingAttachmentInfos.push_back({
        *depthStencilAttachment->view, VULKAN_HPP_NAMESPACE::ImageLayout::eDepthStencilAttachmentOptimal,
        {}, {}, {},
        depthStencilAttachmentInfo.loadOp, depthStencilAttachmentInfo.storeOp, depthStencilAttachmentInfo.clearValue,
    });
    assert(swapchainAttachmentPushed && "swapchainImageIndex set but there is no swapchain attachment in the attachment group.");

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