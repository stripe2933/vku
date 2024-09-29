module;

#include <macros.hpp>

#include <cassert>
#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <algorithm>
#include <bit>
#include <ranges>
#include <vector>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:images.Image;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vulkan_hpp;
import :utils;

#ifdef NDEBUG
#define NOEXCEPT_IF_RELEASE noexcept
#else
#define NOEXCEPT_IF_RELEASE
#endif

namespace vku {
    /**
     * A thin wrapper around <tt>vk::Image</tt> with additional information such as extent, format, mip levels and array
     * layers. It also provides some utility functions to work with images. You can construct this struct from an existing
     * <tt>vk::Image</tt>.
     */
    export struct Image {
        VULKAN_HPP_NAMESPACE::Image image;
        VULKAN_HPP_NAMESPACE::Extent3D extent;
        VULKAN_HPP_NAMESPACE::Format format;
        std::uint32_t mipLevels;
        std::uint32_t arrayLayers;

        // --------------------
        // User-defined conversion functions.
        // --------------------

        [[nodiscard]] operator VULKAN_HPP_NAMESPACE::Image() const noexcept; // can be implicitly converted to vk::Image.

        // --------------------
        // Member functions.
        // --------------------

        /**
         * Get <tt>vk::ImageViewCreateInfo</tt> struct with the specified \p type and identity swizzling. Aspect flags are
         * inferred from the image format. Subresource range is set to the full mip levels and array layers of the image.
         * @param type Image view type (default=<tt>vk::ImageViewType::e2D<tt>).
         * @return <tt>vk::ImageViewCreateInfo</tt> struct.
         * @note See <tt>inferAspectFlags(vk::Format)</tt> for aspect flags inference rule.
         * @note It internally asserts format is not <tt>vk::Format::eUndefined</tt> for debug (<tt>NDEBUG</tt> is not defined)
         * environment.
         * @note It internally calls <tt>inferAspectFlags(vk::Format)</tt> and <tt>getViewCreateInfo(const vk::ImageSubresourceRange&, vk::ImageViewType)</tt>.
         */
        [[nodiscard]] auto getViewCreateInfo(VULKAN_HPP_NAMESPACE::ImageViewType type = VULKAN_HPP_NAMESPACE::ImageViewType::e2D) const NOEXCEPT_IF_RELEASE -> VULKAN_HPP_NAMESPACE::ImageViewCreateInfo;

        /**
         * Get <tt>vk::ImageViewCreateInfo</tt> struct with the specified \p subresourceRange, \p type and identity
         * swizzling.
         * @param subresourceRange Image subresource range.
         * @param type <tt>vk::ImageViewCreateInfo</tt> struct.
         * @return <tt>vk::ImageViewCreateInfo</tt> struct.
         * @note You can use <tt>getViewCreateInfo(vk::ImageViewType)</tt> convenience function for full subresource
         * range with automatically inferred aspect flags from the format.
         */
        [[nodiscard]] auto getViewCreateInfo(const VULKAN_HPP_NAMESPACE::ImageSubresourceRange &subresourceRange, VULKAN_HPP_NAMESPACE::ImageViewType type = VULKAN_HPP_NAMESPACE::ImageViewType::e2D) const noexcept -> VULKAN_HPP_NAMESPACE::ImageViewCreateInfo;

        /**
         * Get <tt>vk::ImageViewCreateInfo</tt> structs for all mip levels with the specified \p type. Aspect flags are
         * inferred from the image format.
         * @param type Image view type (default=<tt>vk::ImageViewType::e2D<tt>).
         * @return Vector of <tt>vk::ImageViewCreateInfo</tt> structs for all mip levels.
         * @note See <tt>inferAspectFlags(vk::Format)</tt> for aspect flags inference rule.
         * @note It internally calls <tt>inferAspectFlags(vk::Format)</tt> and <tt>getViewCreateInfo(const vk::ImageSubresourceRange&, vk::ImageViewType)</tt>.
         */
        [[nodiscard]] auto getMipViewCreateInfos(VULKAN_HPP_NAMESPACE::ImageViewType type = VULKAN_HPP_NAMESPACE::ImageViewType::e2D) const NOEXCEPT_IF_RELEASE -> std::vector<VULKAN_HPP_NAMESPACE::ImageViewCreateInfo> {
            return std::views::iota(0U, mipLevels)
                | std::views::transform([&, aspectFlags = inferAspectFlags(format)](std::uint32_t level) {
                    return VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
                        {},
                        image,
                        type,
                        format,
                        {},
                        { aspectFlags, level, 1, 0, VULKAN_HPP_NAMESPACE::RemainingArrayLayers }
                    };
                })
                | std::ranges::to<std::vector>();
        }

        /**
         * Get the extent of the specified mip level.
         * @param mipLevel Mip level, starts from zero, which must be less than <tt>maxMipLevels(extent)</tt>.
         * @return Mip image extent.
         * @note This will internally call <tt>Image::mipExtent(const vk::Extent2D&, std::uint32_t)</tt> with current image
         * extent. See the function for more details.
         */
        [[nodiscard]] auto mipExtent(std::uint32_t mipLevel) const NOEXCEPT_IF_RELEASE -> VULKAN_HPP_NAMESPACE::Extent2D;

        // --------------------
        // Functions.
        // --------------------

        /**
         * Infer image subresource range's aspect flags from \p format.
         * @param format Format for which aspect flags are inferred.
         * @return Inferred aspect flags.
         * @note Inferring aspect flags from the format is done by the following rules:
         * - format=[<tt>D16Unorm</tt>, <tt>D32Sfloat</tt>] -> Depth
         * - format=[<tt>D16UnormS8Uint</tt>, <tt>D24UnormS8Uint</tt>, <tt>D32SfloatS8Uint</tt>] -> Depth | Stencil
         * - format=[<tt>S8Uint</tt>] -> Stencil
         * - otherwise -> Color.
         */
        [[nodiscard]] static constexpr auto inferAspectFlags(VULKAN_HPP_NAMESPACE::Format format) NOEXCEPT_IF_RELEASE -> VULKAN_HPP_NAMESPACE::ImageAspectFlags;

        /**
         * Get the available maximum mip levels for image that width=height=\p size.
         * @code{.cpp}
         * maxMipLevels(512) -> 10 // 512x512 image can have 10 mip images whose dimensions are 512x512, 256x256, ..., 1x1.
         * @endcode
         * @param size Image dimension both applied to width and height.
         * @return Maximum mip levels.
         * @note It internally asserts \p size is greater than zero for debug (<tt>NDEBUG</tt> is not defined) environment.
         */
        [[nodiscard]] static constexpr auto maxMipLevels(std::uint32_t size) NOEXCEPT_IF_RELEASE -> std::uint32_t;

        /**
         * Get the available maximum mip levels for image with the specified \p extent.
         * @code{.cpp}
         * maxMipLevels({ 512, 256 }) -> 9 // 512x256 image can have 9 mip images whose dimensions are 512x256, 256x128, ..., 2x1.
         * maxMipLevels({ 512, 512 }) -> 10
         * @endcode
         * @param extent Image dimension.
         * @return Maximum mip levels.
         * @note It internally calls <tt>maxMipLevels(std::min(extent.width, extent.height))</tt>.
         * @note maxMipLevels({ width, height }) is equivalent to maxMipLevels(std::min(width, height)).
         */
        [[nodiscard]] static constexpr auto maxMipLevels(const VULKAN_HPP_NAMESPACE::Extent2D &extent) NOEXCEPT_IF_RELEASE -> std::uint32_t;

        /**
         * Get the extent of the specified mip level.
         * @code{.cpp}
         * mipExtent({ 512, 512 }, 3) -> { 64, 64 } // 512x512 image's 3rd (use zero-based indexing) mip image has 64x64 dimension.
         * @endcode
         * @param extent Original image extent.
         * @param mipLevel Mip level, starts from zero, which must be less than <tt>maxMipLevels(extent)</tt>.
         * @return Mip image extent.
         * @note On debug environment (<tt>NDEBUG</tt> is not defined), this function will assert if \p mipLevel is
         * greater than maxMipLevels(\p extent).
         */
        [[nodiscard]] static constexpr auto mipExtent(const VULKAN_HPP_NAMESPACE::Extent2D &extent, std::uint32_t mipLevel) NOEXCEPT_IF_RELEASE -> VULKAN_HPP_NAMESPACE::Extent2D;
    };

    /**
     * Creating <tt>vk::ImageSubresourceRange</tt> that contains the whole mip levels and array layers, with specified
     * \p aspectFlags.
     * @param aspectFlags Image aspect. Default is <tt>vk::ImageAspectFlagBits::eColor</tt>.
     * @return <tt>vk::ImageSubresourceRange</tt> that contains the whole mip levels and array layers.
     */
    export
    [[nodiscard]] constexpr auto fullSubresourceRange(VULKAN_HPP_NAMESPACE::ImageAspectFlags aspectFlags = VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eColor) noexcept -> VULKAN_HPP_NAMESPACE::ImageSubresourceRange {
        return { aspectFlags, 0, VULKAN_HPP_NAMESPACE::RemainingMipLevels, 0, VULKAN_HPP_NAMESPACE::RemainingArrayLayers };
    }
}

// --------------------
// Implementations.
// --------------------

vku::Image::operator VULKAN_HPP_NAMESPACE::Image() const noexcept {
    return image;
}

auto vku::Image::getViewCreateInfo(
    VULKAN_HPP_NAMESPACE::ImageViewType type
) const NOEXCEPT_IF_RELEASE -> VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
    assert(format != VULKAN_HPP_NAMESPACE::Format::eUndefined && "Cannot infer image aspect flags from undefined format");
    return getViewCreateInfo(fullSubresourceRange(inferAspectFlags(format)), type);
}

auto vku::Image::getViewCreateInfo(
    const VULKAN_HPP_NAMESPACE::ImageSubresourceRange &subresourceRange,
    VULKAN_HPP_NAMESPACE::ImageViewType type
) const noexcept -> VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
    return { {}, image, type, format, {}, subresourceRange };
}

auto vku::Image::mipExtent(
    std::uint32_t mipLevel
) const NOEXCEPT_IF_RELEASE -> VULKAN_HPP_NAMESPACE::Extent2D {
    return mipExtent({ extent.width, extent.height }, mipLevel);
}

constexpr auto vku::Image::inferAspectFlags(
    VULKAN_HPP_NAMESPACE::Format format
) NOEXCEPT_IF_RELEASE -> VULKAN_HPP_NAMESPACE::ImageAspectFlags {
    switch (format) {
        case VULKAN_HPP_NAMESPACE::Format::eD16Unorm:
        case VULKAN_HPP_NAMESPACE::Format::eD32Sfloat:
            return VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eDepth;
        case VULKAN_HPP_NAMESPACE::Format::eD16UnormS8Uint:
        case VULKAN_HPP_NAMESPACE::Format::eD24UnormS8Uint:
        case VULKAN_HPP_NAMESPACE::Format::eD32SfloatS8Uint:
            return VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eDepth | VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eStencil;
        case VULKAN_HPP_NAMESPACE::Format::eS8Uint:
            return VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eStencil;
        default:
            return VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eColor;
    }
}

constexpr auto vku::Image::maxMipLevels(
    std::uint32_t size
) NOEXCEPT_IF_RELEASE -> std::uint32_t {
    assert(size > 0U && "size must be greater than zero");
    return std::bit_width(size);
}

constexpr auto vku::Image::maxMipLevels(
    const VULKAN_HPP_NAMESPACE::Extent2D &extent
) NOEXCEPT_IF_RELEASE -> std::uint32_t {
    return maxMipLevels(std::min(extent.width, extent.height));
}

constexpr auto vku::Image::mipExtent(
    const VULKAN_HPP_NAMESPACE::Extent2D &extent,
    std::uint32_t mipLevel
) NOEXCEPT_IF_RELEASE -> VULKAN_HPP_NAMESPACE::Extent2D {
    assert(mipLevel < maxMipLevels(extent) && "mipLevel must be less than maxMipLevels(extent)");
    return { extent.width >> mipLevel, extent.height >> mipLevel };
}