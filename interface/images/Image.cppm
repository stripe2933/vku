/** @file images/Image.cppm
 */

module;

#include <cassert>

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:images.Image;

import std;
export import vulkan_hpp;
import :utils;

#ifdef NDEBUG
#define NOEXCEPT_IF_RELEASE noexcept
#else
#define NOEXCEPT_IF_RELEASE
#endif

// This macro is for Clang's false-positive build error when using lambda in initializer of non-inline variable in named
// modules. We can specify the inline keyword to workaround this.
// See: https://github.com/llvm/llvm-project/issues/110146
// TODO: remove this macro when the issue fixed.
#if __clang__
#define CLANG_INLINE inline
#else
#define CLANG_INLINE
#endif

namespace vku {
    /**
     * @brief Non-owning <tt>vk::Image</tt> handle with the additional information.
     */
    export struct Image {
        /**
         * @brief Vulkan handle that represents the image.
         */
        VULKAN_HPP_NAMESPACE::Image image;

        /**
         * @brief 3-dimensional extent of the image.
         */
        VULKAN_HPP_NAMESPACE::Extent3D extent;

        /**
         * @brief Format of the image.
         */
        VULKAN_HPP_NAMESPACE::Format format;

        /**
         * @brief Number of mip levels of the image.
         */
        std::uint32_t mipLevels;

        /**
         * @brief Number of array layers of the image.
         */
        std::uint32_t arrayLayers;

        // --------------------
        // User-defined conversion functions.
        // --------------------

        /**
         * Make this struct implicitly convertible to <tt>vk::Image</tt>.
         */
        [[nodiscard]] operator VULKAN_HPP_NAMESPACE::Image() const noexcept {
            return image;
        }

        // --------------------
        // Member functions.
        // --------------------

        /**
         * @brief <tt>vk::ImageViewCreateInfo</tt> struct with the specified \p type and identity swizzling.
         *
         * Aspect flags are inferred from the image format.
         * Subresource range uses all mip levels and array layers of the image.
         *
         * @param type Image view type used for create info.
         * @return <tt>vk::ImageViewCreateInfo</tt> struct.
         */
        [[nodiscard]] VULKAN_HPP_NAMESPACE::ImageViewCreateInfo getViewCreateInfo(
            VULKAN_HPP_NAMESPACE::ImageViewType type = VULKAN_HPP_NAMESPACE::ImageViewType::e2D
        ) const NOEXCEPT_IF_RELEASE {
            return getViewCreateInfo({ inferAspectFlags(format), 0, VULKAN_HPP_NAMESPACE::RemainingMipLevels, 0, VULKAN_HPP_NAMESPACE::RemainingArrayLayers }, type);
        }

        /**
         * @brief <tt>vk::ImageViewCreateInfo</tt> struct with the specified \p subresourceRange, \p type and identity swizzling.
         * @param subresourceRange vk::ImageSubresourceRange Image subresource range.
         * @param type vk::ImageViewCreateInfo create info struct.
         * @return <tt>vk::ImageViewCreateInfo</tt> struct.
         * @note You can use <tt>getViewCreateInfo(vk::ImageViewType)</tt> method for convenience (full subresource
         * range with automatically inferred aspect flags from the format).
         */
        [[nodiscard]] VULKAN_HPP_NAMESPACE::ImageViewCreateInfo getViewCreateInfo(
            const VULKAN_HPP_NAMESPACE::ImageSubresourceRange &subresourceRange,
            VULKAN_HPP_NAMESPACE::ImageViewType type = VULKAN_HPP_NAMESPACE::ImageViewType::e2D
        ) const noexcept {
            return { {}, image, type, format, {}, subresourceRange };
        }

        /**
         * @brief Range of <tt>vk::ImageViewCreateInfo</tt> structs for each mip levels with the specified \p type.
         *
         * Aspect flags are inferred from the image format.
         *
         * @param type Image view type (default=<tt>vk::ImageViewType::e2D</tt>).
         * @return Range of <tt>vk::ImageViewCreateInfo</tt> structs for each mip levels.
         */
        [[nodiscard]] CLANG_INLINE auto getMipViewCreateInfos(VULKAN_HPP_NAMESPACE::ImageViewType type = VULKAN_HPP_NAMESPACE::ImageViewType::e2D) const NOEXCEPT_IF_RELEASE {
            return std::views::iota(0U, mipLevels)
                | std::views::transform([this, type, aspectFlags = inferAspectFlags(format)](std::uint32_t level) {
                    return VULKAN_HPP_NAMESPACE::ImageViewCreateInfo {
                        {},
                        image,
                        type,
                        format,
                        {},
                        { aspectFlags, level, 1, 0, VULKAN_HPP_NAMESPACE::RemainingArrayLayers },
                    };
                });
        }

        /**
         * @brief 3-dimensional extent of the specified mip level.
         * @param mipLevel Mip level, starts from zero. Must be less than <tt>maxMipLevels(extent)</tt>.
         * @return Mip image extent.
         */
        [[nodiscard]] VULKAN_HPP_NAMESPACE::Extent3D mipExtent(std::uint32_t mipLevel) const NOEXCEPT_IF_RELEASE {
            return mipExtent(extent, mipLevel);
        }

        /**
         * @brief 2-dimensional extent of the specified mip level.
         * @param mipLevel Mip level, starts from zero. Must be less than <tt>maxMipLevels({ )</tt>.
         * @return Mip image extent.
         */
        [[nodiscard]] VULKAN_HPP_NAMESPACE::Extent2D mipExtent2D(std::uint32_t mipLevel) const NOEXCEPT_IF_RELEASE {
            return mipExtent(VULKAN_HPP_NAMESPACE::Extent2D { extent.width, extent.height }, mipLevel);
        }

        // --------------------
        // Functions.
        // --------------------

        /**
         * @brief Inferred image aspect flags from \p format.
         *
         * Inferring is done by the following rules:
         * - <tt>D16Unorm</tt>, <tt>D32Sfloat</tt> -> Depth
         * - <tt>D16UnormS8Uint</tt>, <tt>D24UnormS8Uint</tt>, <tt>D32SfloatS8Uint</tt> -> Depth | Stencil
         * - <tt>S8Uint</tt> -> Stencil
         * - otherwise -> Color.
         *
         * @param format Format used for image aspect flags inferring. Must not be <tt>vk::Format::eUndefined</tt>.
         * @return Inferred aspect flags.
         */
        [[nodiscard]] static constexpr VULKAN_HPP_NAMESPACE::ImageAspectFlags inferAspectFlags(VULKAN_HPP_NAMESPACE::Format format) NOEXCEPT_IF_RELEASE {
            switch (format) {
                case VULKAN_HPP_NAMESPACE::Format::eUndefined:
                    std::unreachable();
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

        /**
         * @brief Available maximum mip levels for image whose extent is (\p size, \p size, \p size).
         * @code{.cpp}
         * maxMipLevels(512) -> 10 // 512x512x512 image can have 10 mip images whose dimensions are 512x512x512, 256x256x256, ..., 1x1x1.
         * @endcode
         * @param size Image dimension both applied to width, height and depth. Must be nonzero.
         * @return Maximum mip levels following the Vulkan specification.
         */
        [[nodiscard]] static constexpr std::uint32_t maxMipLevels(std::uint32_t size) NOEXCEPT_IF_RELEASE {
            assert(size > 0U && "size must be greater than zero");
            return std::bit_width(size);
        }

        /**
         * @brief Available maximum mip levels for image with the specified \p extent.
         * @code{.cpp}
         * maxMipLevels({ 512, 512 }) -> 10
         * maxMipLevels({ 512, 256 }) -> 10 // 512x256 image can have 9 mip images whose dimensions are 512x256, 256x128, ..., 2x1, 1x1.
         * @endcode
         * @param extent 2-dimensional image extent.
         * @return Maximum mip levels following the Vulkan specification.
         */
        [[nodiscard]] static constexpr std::uint32_t maxMipLevels(const VULKAN_HPP_NAMESPACE::Extent2D &extent) NOEXCEPT_IF_RELEASE {
            return maxMipLevels(std::max(extent.width, extent.height));
        }

        /**
         * @brief Available maximum mip levels for image with the specified \p extent.
         * @code{.cpp}
         * maxMipLevels({ 512, 512, 512 }) -> 10
         * maxMipLevels({ 512, 256, 4 }) -> 10 // 512x256x4 image can have 9 mip images whose dimensions are 512x256x4, 256x128x2, ..., 2x1x1, 1x1x1.
         * @endcode
         * @param extent 3-dimensional image extent.
         * @return Maximum mip levels following the Vulkan specification.
         */
        [[nodiscard]] static constexpr std::uint32_t maxMipLevels(const VULKAN_HPP_NAMESPACE::Extent3D &extent) NOEXCEPT_IF_RELEASE {
            return maxMipLevels(std::max({ extent.width, extent.height, extent.depth }));
        }

        /**
         * @brief 2-dimensional extent of the specified mip level.
         * @code{.cpp}
         * mipExtent({ 512, 512 }, 3) -> { 64, 64 } // 512x512 image's 3rd (use zero-based indexing) mip image has 64x64 dimension.
         * mipExtent({ 256, 512 }, 8) -> { 1, 2 }
         * mipExtent({ 256, 512 }, 9) -> { 1, 1 } // min(256 >> 9, 1) = min(0, 1) = 1
         * @endcode
         * @param extent Original image extent.
         * @param mipLevel Mip level, starts from zero. Must be less than <tt>maxMipLevels(extent)</tt>.
         * @return Mip image extent.
         */
        [[nodiscard]] static constexpr VULKAN_HPP_NAMESPACE::Extent2D mipExtent(
            const VULKAN_HPP_NAMESPACE::Extent2D &extent,
            std::uint32_t mipLevel
        ) NOEXCEPT_IF_RELEASE {
            assert(mipLevel < maxMipLevels(extent) && "mipLevel must be less than maxMipLevels(extent)");
            return { std::max(extent.width >> mipLevel, 1U), std::max(extent.height >> mipLevel, 1U) };
        }

        /**
         * @brief 3-dimensional extent of the specified mip level.
         * @code{.cpp}
         * mipExtent({ 512, 512, 16 }, 3) -> { 64, 64, 2 } // 512x512x16 image's 3rd (use zero-based indexing) mip image has 64x64x2 dimension.
         * mipExtent({ 256, 512, 16 }, 4) -> { 16, 32, 1 }
         * mipExtent({ 256, 512, 16 }, 9) -> { 1, 1, 1 } // min(256 >> 9, 1) = min(0, 1) = 1, min(16 >> 9, 1) = min(0, 1) = 1
         * @endcode
         * @param extent Original image extent.
         * @param mipLevel Mip level, starts from zero. Must be less than <tt>maxMipLevels(extent)</tt>.
         * @return Mip image extent.
         */
        [[nodiscard]] static constexpr VULKAN_HPP_NAMESPACE::Extent3D mipExtent(
            const VULKAN_HPP_NAMESPACE::Extent3D &extent,
            std::uint32_t mipLevel
        ) NOEXCEPT_IF_RELEASE {
            assert(mipLevel < maxMipLevels(extent) && "mipLevel must be less than maxMipLevels(extent)");
            return { std::max(extent.width >> mipLevel, 1U), std::max(extent.height >> mipLevel, 1U), std::max(extent.depth >> mipLevel, 1U) };
        }
    };

    /**
     * @brief Creating <tt>vk::ImageSubresourceRange</tt> that contains the whole mip levels and array layers, with specified \p aspectFlags.
     * @param aspectFlags Image aspect. Default is <tt>vk::ImageAspectFlagBits::eColor</tt>.
     * @return <tt>vk::ImageSubresourceRange</tt> that contains the whole mip levels and array layers.
     */
    export
    [[nodiscard]] constexpr VULKAN_HPP_NAMESPACE::ImageSubresourceRange fullSubresourceRange(
        VULKAN_HPP_NAMESPACE::ImageAspectFlags aspectFlags = VULKAN_HPP_NAMESPACE::ImageAspectFlagBits::eColor
    ) noexcept {
        return { aspectFlags, 0, VULKAN_HPP_NAMESPACE::RemainingMipLevels, 0, VULKAN_HPP_NAMESPACE::RemainingArrayLayers };
    }
}