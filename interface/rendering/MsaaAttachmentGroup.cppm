module;

#include <cassert>
#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <algorithm>
#include <optional>
#include <ranges>
#include <span>
#include <variant>
#include <vector>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:rendering.MsaaAttachmentGroup;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
import :details.functional;
export import :rendering.Attachment;
import :rendering.AttachmentGroupBase;
export import :rendering.MsaaAttachment;
import :utils.RefHolder;

// #define VMA_HPP_NAMESPACE to vma, if not defined.
#ifndef VMA_HPP_NAMESPACE
#define VMA_HPP_NAMESPACE vma
#endif

namespace vku {
    export struct MsaaAttachmentGroup : AttachmentGroupBase {
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
        std::vector<std::variant<MsaaAttachment, SwapchainMsaaAttachment>> colorAttachments;
        std::optional<Attachment> depthStencilAttachment;

        explicit MsaaAttachmentGroup(const VULKAN_HPP_NAMESPACE::Extent2D &extent, VULKAN_HPP_NAMESPACE::SampleCountFlagBits sampleCount);
        MsaaAttachmentGroup(const MsaaAttachmentGroup&) = delete;
        MsaaAttachmentGroup(MsaaAttachmentGroup&&) noexcept = default;
        auto operator=(const MsaaAttachmentGroup&) -> MsaaAttachmentGroup& = delete;
        auto operator=(MsaaAttachmentGroup&&) noexcept -> MsaaAttachmentGroup& = default;
        ~MsaaAttachmentGroup() override = default;

        auto addColorAttachment(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
            const Image &image,
            const Image &resolveImage,
            VULKAN_HPP_NAMESPACE::Format viewFormat = {}
        ) -> const MsaaAttachment&;
        auto addColorAttachment(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
            const Image &image,
            const Image &resolveImage,
            const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &viewCreateInfo,
            const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &resolveViewCreateInfo
        ) -> const MsaaAttachment&;
        auto addSwapchainAttachment(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
            const Image &image,
            std::span<const VULKAN_HPP_NAMESPACE::Image> swapchainImages,
            VULKAN_HPP_NAMESPACE::Format viewFormat
        ) -> const SwapchainMsaaAttachment&;
        auto addSwapchainAttachment(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
            const Image &image,
            std::span<const VULKAN_HPP_NAMESPACE::Image> swapchainImages,
            const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &viewCreateInfo
        ) -> const SwapchainMsaaAttachment&;
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
         * without needing to use the more cumbersome <tt>get<vku::MsaaAttachment>(...)</tt> call.
         * @note This function is considered "unsafe" because if the element at the specified index isn't a <tt>vku::MsaaAttachment</tt>,
         * it can lead to undefined behavior.
         */
        [[nodiscard]] auto getColorAttachment(std::size_t index) const noexcept -> const MsaaAttachment& {
            return *get_if<MsaaAttachment>(&colorAttachments[index]);
        }

        /**
         * Get the const reference of swapchain attachment at the specified index.
         * @param index The index of the swapchain attachment.
         * @return The const reference of the swapchain attachment.
         * @note This function is designed to make it easier for users to access elements from <tt>attachmentGroup.colorAttachments[i]</tt>
         * without needing to use the more cumbersome <tt>get<vku::SwapchainMsaaAttachment>(...)</tt> call.
         * @note This function is considered "unsafe" because if the element at the specified index isn't a <tt>vku::SwapchainMsaaAttachment</tt>,
         * it can lead to undefined behavior.
         */
        [[nodiscard]] auto getSwapchainAttachment(std::size_t index) const noexcept -> const SwapchainMsaaAttachment& {
            return *get_if<SwapchainMsaaAttachment>(&colorAttachments[index]);
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
            std::span<const ColorAttachmentInfo> colorAttachmentInfos
        ) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>>;

        [[nodiscard]] auto getRenderingInfo(
            std::span<const ColorAttachmentInfo> colorAttachmentInfos,
            std::uint32_t swapchainImageIndex
        ) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>>;

        [[nodiscard]] auto getRenderingInfo(
            std::span<const ColorAttachmentInfo> colorAttachmentInfos,
            const DepthStencilAttachmentInfo &depthStencilAttachmentInfo
        ) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>>;

        [[nodiscard]] auto getRenderingInfo(
            std::span<const ColorAttachmentInfo> colorAttachmentInfos,
            const DepthStencilAttachmentInfo &depthStencilAttachmentInfo,
            std::uint32_t swapchainImageIndex
        ) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>>;
    };
}

// --------------------
// Implementations.
// --------------------

vku::MsaaAttachmentGroup::MsaaAttachmentGroup(
    const VULKAN_HPP_NAMESPACE::Extent2D &extent,
    VULKAN_HPP_NAMESPACE::SampleCountFlagBits sampleCount
) : AttachmentGroupBase { extent },
    sampleCount { sampleCount } { }

auto vku::MsaaAttachmentGroup::addColorAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &image,
    const Image &resolveImage,
    VULKAN_HPP_NAMESPACE::Format viewFormat
) -> const MsaaAttachment& {
    return addColorAttachment(
        device,
        image,
        resolveImage,
        VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
            {},
            image,
            VULKAN_HPP_NAMESPACE::ImageViewType::e2D,
            viewFormat == VULKAN_HPP_NAMESPACE::Format::eUndefined ? image.format : viewFormat,
            {},
            { VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
        },
        VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
            {},
            resolveImage,
            VULKAN_HPP_NAMESPACE::ImageViewType::e2D,
            viewFormat == VULKAN_HPP_NAMESPACE::Format::eUndefined ? resolveImage.format : viewFormat,
            {},
            { VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
        });
}

auto vku::MsaaAttachmentGroup::addColorAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &image,
    const Image &resolveImage,
    const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &viewCreateInfo,
    const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &resolveViewCreateInfo
) -> const MsaaAttachment & {
    return *get_if<MsaaAttachment>(&colorAttachments.emplace_back(
        std::in_place_type<MsaaAttachment>,
        image,
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView { device, viewCreateInfo },
        resolveImage,
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView { device, resolveViewCreateInfo }));
}

auto vku::MsaaAttachmentGroup::addSwapchainAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &image,
    std::span<const VULKAN_HPP_NAMESPACE::Image> swapchainImages,
    VULKAN_HPP_NAMESPACE::Format viewFormat
) -> const SwapchainMsaaAttachment& {
    return addSwapchainAttachment(
        device,
        image,
        swapchainImages,
        VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
            {},
            image,
            VULKAN_HPP_NAMESPACE::ImageViewType::e2D,
            viewFormat,
            {},
            { VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
        });
}

auto vku::MsaaAttachmentGroup::addSwapchainAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &image,
    std::span<const VULKAN_HPP_NAMESPACE::Image> swapchainImages,
    const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &viewCreateInfo
) -> const SwapchainMsaaAttachment& {
    std::vector<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView> resolveViews;
    resolveViews.reserve(swapchainImages.size());
    for (VULKAN_HPP_NAMESPACE::Image swapchainImage : swapchainImages) {
        resolveViews.emplace_back(device, VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
            {},
            swapchainImage,
            VULKAN_HPP_NAMESPACE::ImageViewType::e2D,
            viewCreateInfo.format,
            {},
            { VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
        });
    }

    return *get_if<SwapchainMsaaAttachment>(&colorAttachments.emplace_back(
        std::in_place_type<SwapchainMsaaAttachment>,
        image,
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView { device, viewCreateInfo },
        std::move(resolveViews)));
}

auto vku::MsaaAttachmentGroup::setDepthStencilAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &image,
    VULKAN_HPP_NAMESPACE::Format viewFormat
) -> const Attachment& {
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

auto vku::MsaaAttachmentGroup::setDepthStencilAttachment(
    const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
    const Image &image,
    const VULKAN_HPP_NAMESPACE::ImageViewCreateInfo &viewCreateInfo
) -> const Attachment & {
    return depthStencilAttachment.emplace(image, VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ImageView { device, viewCreateInfo });
}

auto vku::MsaaAttachmentGroup::createColorImage(
    VMA_HPP_NAMESPACE::Allocator allocator,
    VULKAN_HPP_NAMESPACE::Format format,
    VULKAN_HPP_NAMESPACE::ImageUsageFlags usage,
    const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
) const -> AllocatedImage {
    return createAttachmentImage(allocator, format, sampleCount, usage, allocationCreateInfo);
}

auto vku::MsaaAttachmentGroup::createResolveImage(
    VMA_HPP_NAMESPACE::Allocator allocator,
    VULKAN_HPP_NAMESPACE::Format viewFormat,
    VULKAN_HPP_NAMESPACE::ImageUsageFlags usage,
    const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
) const -> AllocatedImage {
    return createAttachmentImage(allocator, viewFormat, VULKAN_HPP_NAMESPACE::SampleCountFlagBits::e1, usage, allocationCreateInfo);
}

auto vku::MsaaAttachmentGroup::createDepthStencilImage(
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

auto vku::MsaaAttachmentGroup::getRenderingInfo(
    std::span<const ColorAttachmentInfo> colorAttachmentInfos
) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(!depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");

    std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> renderingAttachmentInfos;
    renderingAttachmentInfos.reserve(colorAttachmentInfos.size());
    for (const auto &[attachment, info] : std::views::zip(colorAttachments, colorAttachmentInfos)) {
        auto *const pMsaaAttachment = get_if<MsaaAttachment>(&attachment);
        assert(pMsaaAttachment && "More than one SwapchainMsaaAttachment in the attachment group.");
        renderingAttachmentInfos.push_back({
            *pMsaaAttachment->view, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
            info.resolveMode, *pMsaaAttachment->resolveView, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
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

auto vku::MsaaAttachmentGroup::getRenderingInfo(
    std::span<const ColorAttachmentInfo> colorAttachmentInfos,
    std::uint32_t swapchainImageIndex
) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(!depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");

    std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> renderingAttachmentInfos;
    renderingAttachmentInfos.reserve(colorAttachmentInfos.size());

    // SwapchainMsaaAttachment can only appear once in the attachment group. Therefore, if a SwapchainMsaaAttachment
    // already push_backed into renderingAttachmentInfos, explicit std::visit call for the rest variants is not necessary.
    bool swapchainAttachmentPushed = false;
    for (const auto &[attachment, info] : std::views::zip(colorAttachments, colorAttachmentInfos)) {
        const auto [view, resolveView] = [&]() {
            if (swapchainAttachmentPushed) {
                auto *pMsaaAttachment = get_if<MsaaAttachment>(&attachment);
                assert(pMsaaAttachment && "More than one SwapchainMsaaAttachment in the attachment group.");
                return std::pair { *pMsaaAttachment->view, *pMsaaAttachment->resolveView };
            }

            return visit(multilambda {
                [](const MsaaAttachment &msaaAttachment) {
                    return std::pair { *msaaAttachment.view, *msaaAttachment.resolveView };
                },
                [&](const SwapchainMsaaAttachment &swapchainMsaaAttachment) {
                    swapchainAttachmentPushed = true;
                    return std::pair { *swapchainMsaaAttachment.view, *swapchainMsaaAttachment.resolveViews[swapchainImageIndex] };
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

auto vku::MsaaAttachmentGroup::getRenderingInfo(
    std::span<const ColorAttachmentInfo> colorAttachmentInfos,
    const DepthStencilAttachmentInfo &depthStencilAttachmentInfo
) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");

    std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> renderingAttachmentInfos;
    renderingAttachmentInfos.reserve(colorAttachmentInfos.size() + 1);
    for (const auto &[attachment, info] : std::views::zip(colorAttachments, colorAttachmentInfos)) {
        auto *const pMsaaAttachment = get_if<MsaaAttachment>(&attachment);
        assert(pMsaaAttachment && "More than one SwapchainMsaaAttachment in the attachment group.");
        renderingAttachmentInfos.push_back({
            *pMsaaAttachment->view, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
            info.resolveMode, *pMsaaAttachment->resolveView, VULKAN_HPP_NAMESPACE::ImageLayout::eColorAttachmentOptimal,
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

auto vku::MsaaAttachmentGroup::getRenderingInfo(
    std::span<const ColorAttachmentInfo> colorAttachmentInfos,
    const DepthStencilAttachmentInfo &depthStencilAttachmentInfo,
    std::uint32_t swapchainImageIndex
) const -> RefHolder<VULKAN_HPP_NAMESPACE::RenderingInfo, std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo>> {
    assert(colorAttachments.size() == colorAttachmentInfos.size() && "Color attachment info count mismatch");
    assert(depthStencilAttachment.has_value() && "Depth-stencil attachment info mismatch");

    std::vector<VULKAN_HPP_NAMESPACE::RenderingAttachmentInfo> renderingAttachmentInfos;
    renderingAttachmentInfos.reserve(colorAttachmentInfos.size() + 1);

    // SwapchainMsaaAttachment can only appear once in the attachment group. Therefore, if a SwapchainMsaaAttachment
    // already push_backed into renderingAttachmentInfos, explicit std::visit call for the rest variants is not necessary.
    bool swapchainAttachmentPushed = false;
    for (const auto &[attachment, info] : std::views::zip(colorAttachments, colorAttachmentInfos)) {
        const auto [view, resolveView] = [&]() {
            if (swapchainAttachmentPushed) {
                auto *pMsaaAttachment = get_if<MsaaAttachment>(&attachment);
                assert(pMsaaAttachment && "More than one SwapchainMsaaAttachment in the attachment group.");
                return std::pair { *pMsaaAttachment->view, *pMsaaAttachment->resolveView };
            }

            return visit(multilambda {
                [](const MsaaAttachment &msaaAttachment) {
                    return std::pair { *msaaAttachment.view, *msaaAttachment.resolveView };
                },
                [&](const SwapchainMsaaAttachment &swapchainMsaaAttachment) {
                    swapchainAttachmentPushed = true;
                    return std::pair { *swapchainMsaaAttachment.view, *swapchainMsaaAttachment.resolveViews[swapchainImageIndex] };
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