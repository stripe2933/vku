#pragma once

#include <bit>

#include <vulkan/vulkan_raii.hpp>
#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>

#include "utils.hpp"

namespace vku {
    struct Image {
        vk::Image image;
        vk::Extent3D extent;
        vk::Format format;
        std::uint32_t mipLevels;
        std::uint32_t arrayLayers;

        constexpr operator vk::Image() const noexcept {
            return image;
        }

        [[nodiscard]] constexpr auto maxMipLevels() const noexcept -> vk::Extent2D {
            return maxMipLevels(convertExtent2D(extent));
        }

        [[nodiscard]] constexpr auto mipExtent(
            std::uint32_t mipLevel
        ) const noexcept -> vk::Extent2D {
            return mipExtent(convertExtent2D(extent), mipLevel);
        }

        [[nodiscard]] static constexpr auto maxMipLevels(
            std::uint32_t size
        ) noexcept -> std::uint32_t {
            return std::bit_width(size);
        }

        [[nodiscard]] static constexpr auto maxMipLevels(
            const vk::Extent2D &extent
        ) noexcept -> std::uint32_t {
            return maxMipLevels(std::min(extent.width, extent.height));
        }

        [[nodiscard]] static constexpr auto mipExtent(
            const vk::Extent2D &extent,
            std::uint32_t mipLevel
        ) noexcept -> vk::Extent2D {
            return { extent.width >> mipLevel, extent.height >> mipLevel };
        }
    };

    struct AllocatedImage : Image {
        vma::Allocator allocator;
        vma::Allocation allocation;

        AllocatedImage(
            vma::Allocator allocator,
            const vk::ImageCreateInfo &createInfo,
            const vma::AllocationCreateInfo &allocationCreateInfo
        );
        AllocatedImage(const AllocatedImage&) = delete;
        AllocatedImage(AllocatedImage &&src) noexcept;
        auto operator=(const AllocatedImage&) -> AllocatedImage& = delete;
        auto operator=(AllocatedImage &&src) noexcept -> AllocatedImage&;
        ~AllocatedImage();
    };
}