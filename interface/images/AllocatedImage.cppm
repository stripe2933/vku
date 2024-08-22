module;

#ifndef VKU_USE_STD_MODULE
#include <tuple>
#include <utility>
#ifdef _MSC_VER
#include <string_view>
#endif
#endif

export module vku:images.AllocatedImage;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vk_mem_alloc_hpp;
export import :images.Image;

// #define VMA_HPP_NAMESPACE to vma, if not defined.
#ifndef VMA_HPP_NAMESPACE
#define VMA_HPP_NAMESPACE vma
#endif

namespace vku {
    export struct AllocatedImage : Image {
        VMA_HPP_NAMESPACE::Allocator allocator;
        VMA_HPP_NAMESPACE::Allocation allocation;

        AllocatedImage(
            VMA_HPP_NAMESPACE::Allocator allocator,
            const vk::ImageCreateInfo &createInfo,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = { {}, VMA_HPP_NAMESPACE::MemoryUsage::eAutoPreferDevice });
        AllocatedImage(const AllocatedImage&) = delete;
        AllocatedImage(AllocatedImage &&src) noexcept;
        auto operator=(const AllocatedImage&) -> AllocatedImage& = delete;
        auto operator=(AllocatedImage &&src) noexcept -> AllocatedImage&;
        virtual ~AllocatedImage();
    };
}

// --------------------
// Implementations.
// --------------------

vku::AllocatedImage::AllocatedImage(
    VMA_HPP_NAMESPACE::Allocator allocator,
    const vk::ImageCreateInfo &createInfo,
    const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
) : Image { nullptr, createInfo.extent, createInfo.format, createInfo.mipLevels, createInfo.arrayLayers },
   allocator { allocator } {
    std::tie(image, allocation) = allocator.createImage(createInfo, allocationCreateInfo);
}

vku::AllocatedImage::AllocatedImage(
    AllocatedImage &&src
) noexcept : Image { static_cast<Image>(src) },
             allocator { src.allocator },
             allocation { std::exchange(src.allocation, nullptr) } { }

auto vku::AllocatedImage::operator=(
    AllocatedImage &&src
) noexcept -> AllocatedImage & {
    if (allocation) {
        allocator.destroyImage(image, allocation);
    }

    static_cast<Image &>(*this) = static_cast<Image>(src);
    allocator = src.allocator;
    allocation = std::exchange(src.allocation, nullptr);
    return *this;
}

vku::AllocatedImage::~AllocatedImage() {
    if (allocation) {
        allocator.destroyImage(image, allocation);
    }
}