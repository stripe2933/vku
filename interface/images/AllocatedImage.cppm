/** @file images/AllocatedImage.cppm
 */

module;

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:images.AllocatedImage;

import std;
export import vk_mem_alloc_hpp;
export import :images.Image;

// #define VMA_HPP_NAMESPACE to vma, if not defined.
#ifndef VMA_HPP_NAMESPACE
#define VMA_HPP_NAMESPACE vma
#endif

namespace vku {
    /**
     * @brief Owning image handle with memory allocation.
     */
    export struct AllocatedImage : Image {
        /**
         * @brief VMA allocator used by the creation.
         */

        VMA_HPP_NAMESPACE::Allocator allocator;
        /**
         * @brief Allocation object.
         */
        VMA_HPP_NAMESPACE::Allocation allocation;

        /**
         * @brief Create <tt>vk::Image</tt>, allocate memory, then bind them.
         * @param allocator VMA allocator used to allocate memory.
         * @param createInfo Image create info.
         * @param allocationCreateInfo Allocation create info.
         */
        AllocatedImage(
            VMA_HPP_NAMESPACE::Allocator allocator,
            const VULKAN_HPP_NAMESPACE::ImageCreateInfo &createInfo,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = { {}, VMA_HPP_NAMESPACE::MemoryUsage::eAutoPreferDevice }
        ) : Image { nullptr, createInfo.extent, createInfo.format, createInfo.mipLevels, createInfo.arrayLayers },
            allocator { allocator } {
            std::tie(image, allocation) = allocator.createImage(createInfo, allocationCreateInfo);
        }

        AllocatedImage(const AllocatedImage&) = delete;

        AllocatedImage(AllocatedImage &&src) noexcept
            : Image { static_cast<Image>(src) }
            , allocator { src.allocator }
            , allocation { std::exchange(src.allocation, nullptr) } { }

        AllocatedImage& operator=(const AllocatedImage&) = delete;

        AllocatedImage& operator=(AllocatedImage &&src) noexcept  {
            if (allocation) {
                allocator.destroyImage(image, allocation);
            }

            static_cast<Image &>(*this) = static_cast<Image>(src);
            allocator = src.allocator;
            allocation = std::exchange(src.allocation, nullptr);
            return *this;
        }

        virtual ~AllocatedImage() {
            if (allocation) {
                allocator.destroyImage(image, allocation);
            }
        }
    };
}