module;

#include <cassert>
#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <bit>
#include <algorithm>
#endif

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
     *     */
    export struct Image {
        vk::Image image;
        vk::Extent3D extent;
        vk::Format format;
        std::uint32_t mipLevels;
        std::uint32_t arrayLayers;

        // --------------------
        // User-defined conversion functions.
        // --------------------

        [[nodiscard]] operator vk::Image() const noexcept; // can be implicitly converted to vk::Image.

        // --------------------
        // Member functions.
        // --------------------

        /**
         * Get <tt>vk::ImageViewCreateInfo</tt> struct with the specified \p type and identity swizzling. Aspect flags are
         * inferred from the image format. Subresource range is set to the full mip levels and array layers of the image.
         * @param type Image view type (default=<tt>vk::ImageViewType::e2D<tt>).
         * @return <tt>vk::ImageViewCreateInfo</tt> struct.
         * @note
         * See <tt>inferAspectFlags(vk::Format)</tt> for aspect flags inference rule.<br>
         * It internally asserts format is not <tt>vk::Format::eUndefined</tt> for debug (NDEBUG is not defined) environment.<br>
         * It internally calls <tt>inferAspectFlags(vk::Format)</tt> and <tt>getViewCreateInfo(const vk::ImageSubresourceRange&, vk::ImageViewType)</tt>.
         */
        [[nodiscard]] auto getViewCreateInfo(vk::ImageViewType type = vk::ImageViewType::e2D) const NOEXCEPT_IF_RELEASE -> vk::ImageViewCreateInfo;

        /**
         * Get <tt>vk::ImageViewCreateInfo</tt> struct with the specified \p subresourceRange, \p type and identity
         * swizzling.
         * @param subresourceRange Image subresource range.
         * @param type <tt>vk::ImageViewCreateInfo</tt> struct.
         * @return <tt>vk::ImageViewCreateInfo</tt> struct.
         * @note
         * You can use <tt>getViewCreateInfo(vk::ImageViewType)</tt> convenience function for full subresource range with
         * automatically inferred aspect flags from the format.
         */
        [[nodiscard]] auto getViewCreateInfo(const vk::ImageSubresourceRange &subresourceRange, vk::ImageViewType type = vk::ImageViewType::e2D) const noexcept -> vk::ImageViewCreateInfo;

        /**
         * Get the extent of the specified mip level.
         * @param mipLevel Mip level, starts from zero, which must be less than <tt>maxMipLevels(extent)</tt>.
         * @return Mip image extent.
         * @note
         * This will internally call <tt>Image::mipExtent(const vk::Extent2D&, std::uint32_t)</tt> with current image
         * extent. See the function for more details.
         */
        [[nodiscard]] auto mipExtent(std::uint32_t mipLevel) const NOEXCEPT_IF_RELEASE -> vk::Extent2D;

        // --------------------
        // Functions.
        // --------------------

        /**
         * Infer image subresource range's aspect flags from \p format.
         * @param format Format for which aspect flags are inferred.
         * @return Inferred aspect flags.
         * @note
         * Inferring aspect flags from the format is done by the following rules:
         * - format=[<tt>D16Unorm</tt>, <tt>D32Sfloat</tt>] -> Depth
         * - format=[<tt>D16UnormS8Uint</tt>, <tt>D24UnormS8Uint</tt>, <tt>D32SfloatS8Uint</tt>] -> Depth | Stencil
         * - format=[<tt>S8Uint</tt>] -> Stencil
         * - otherwise -> Color.
         */
        [[nodiscard]] static constexpr auto inferAspectFlags(vk::Format format) NOEXCEPT_IF_RELEASE -> vk::ImageAspectFlags;

        /**
         * Get the available maximum mip levels for image that width=height=\p size.
         * @param size Image dimension both applied to width and height.
         * @return Maximum mip levels.
         * @note
         * It internally asserts \p size is greater than zero for debug (NDEBUG is not defined) environment.
         * @example
         * maxMipLevels(512) -> 10 // 512x512 image can have 10 mip images whose dimensions are 512x512, 256x256, ..., 1x1.
         */
        [[nodiscard]] static constexpr auto maxMipLevels(std::uint32_t size) NOEXCEPT_IF_RELEASE -> std::uint32_t;

        /**
         * Get the available maximum mip levels for image with the specified \p extent.
         * @param extent Image dimension.
         * @return Maximum mip levels.
         * @note
         * It internally calls <tt>maxMipLevels(std::min(extent.width, extent.height))</tt>.
         * @example
         * maxMipLevels({ 512, 256 }) -> 9 // 512x256 image can have 9 mip images whose dimensions are 512x256, 256x128, ..., 2x1.
         * maxMipLevels({ 512, 512 }) -> 10
         * @note maxMipLevels({ width, height }) is equivalent to maxMipLevels(std::min(width, height)).
         */
        [[nodiscard]] static constexpr auto maxMipLevels(const vk::Extent2D &extent) NOEXCEPT_IF_RELEASE -> std::uint32_t;

        /**
         * Get the extent of the specified mip level.
         * @param extent Original image extent.
         * @param mipLevel Mip level, starts from zero, which must be less than <tt>maxMipLevels(extent)</tt>.
         * @return Mip image extent.
         * @example
         * mipExtent({ 512, 512 }, 3) -> { 64, 64 } // 512x512 image's 3rd (use zero-based indexing) mip image has 64x64 dimension.
         * @note On debug environment (NDEBUG is not defined), this function will assert if \p mipLevel is greater than maxMipLevels(\p extent).
         */
        [[nodiscard]] static constexpr auto mipExtent(const vk::Extent2D &extent, std::uint32_t mipLevel) NOEXCEPT_IF_RELEASE -> vk::Extent2D;
    };
}

// --------------------
// Implementations.
// --------------------

vku::Image::operator vk::Image() const noexcept {
    return image;
}

auto vku::Image::getViewCreateInfo(
    vk::ImageViewType type
) const NOEXCEPT_IF_RELEASE -> vk::ImageViewCreateInfo {
    assert(format != vk::Format::eUndefined && "Cannot infer image aspect flags from undefined format");
    return getViewCreateInfo(fullSubresourceRange(inferAspectFlags(format)), type);
}

auto vku::Image::getViewCreateInfo(
    const vk::ImageSubresourceRange &subresourceRange,
    vk::ImageViewType type
) const noexcept -> vk::ImageViewCreateInfo {
    return { {}, image, type, format, {}, subresourceRange };
}

auto vku::Image::mipExtent(
    std::uint32_t mipLevel
) const NOEXCEPT_IF_RELEASE -> vk::Extent2D {
    return mipExtent({ extent.width, extent.height }, mipLevel);
}

constexpr auto vku::Image::inferAspectFlags(
    vk::Format format
) NOEXCEPT_IF_RELEASE -> vk::ImageAspectFlags {
    switch (format) {
        case vk::Format::eD16Unorm:
        case vk::Format::eD32Sfloat:
            return vk::ImageAspectFlagBits::eDepth;
        case vk::Format::eD16UnormS8Uint:
        case vk::Format::eD24UnormS8Uint:
        case vk::Format::eD32SfloatS8Uint:
            return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
        case vk::Format::eS8Uint:
            return vk::ImageAspectFlagBits::eStencil;
        default:
            return vk::ImageAspectFlagBits::eColor;
    }
}

constexpr auto vku::Image::maxMipLevels(
    std::uint32_t size
) NOEXCEPT_IF_RELEASE -> std::uint32_t {
    assert(size > 0U && "size must be greater than zero");
    return std::bit_width(size);
}

constexpr auto vku::Image::maxMipLevels(
    const vk::Extent2D &extent
) NOEXCEPT_IF_RELEASE -> std::uint32_t {
    return maxMipLevels(std::min(extent.width, extent.height));
}

constexpr auto vku::Image::mipExtent(
    const vk::Extent2D &extent,
    std::uint32_t mipLevel
) NOEXCEPT_IF_RELEASE -> vk::Extent2D {
    assert(mipLevel < maxMipLevels(extent) && "mipLevel must be less than maxMipLevels(extent)");
    return { extent.width >> mipLevel, extent.height >> mipLevel };
}