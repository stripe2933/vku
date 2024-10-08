/** @file buffers/AllocatedBuffer.cppm
 */

module;

#ifndef VKU_USE_STD_MODULE
#include <tuple>
#include <utility>
#ifdef _MSC_VER
#include <string_view>
#endif
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:buffers.AllocatedBuffer;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vk_mem_alloc_hpp;
export import :buffers.Buffer;

// #define VMA_HPP_NAMESPACE to vma, if not defined.
#ifndef VMA_HPP_NAMESPACE
#define VMA_HPP_NAMESPACE vma
#endif

namespace vku {
    /**
     * @brief Owning buffer handle with memory allocation.
     */
    export struct AllocatedBuffer : Buffer {
        /**
         * @brief VMA allocator used by the creation.
         */
        VMA_HPP_NAMESPACE::Allocator allocator;

        /**
         * @brief Allocation object.
         */
        VMA_HPP_NAMESPACE::Allocation allocation;

        /**
         * @brief Create <tt>vk::Buffer</tt>, allocate memory, then bind them.
         * @param allocator VMA allocator used to allocate memory.
         * @param createInfo Buffer create info.
         * @param allocationCreateInfo Allocation create info.
         */
        AllocatedBuffer(
            VMA_HPP_NAMESPACE::Allocator allocator,
            const VULKAN_HPP_NAMESPACE::BufferCreateInfo &createInfo,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = { {}, VMA_HPP_NAMESPACE::MemoryUsage::eAutoPreferDevice }
        ) : Buffer { nullptr, createInfo.size },
            allocator { allocator } {
            std::tie(buffer, allocation) = allocator.createBuffer(createInfo, allocationCreateInfo);
        }

        AllocatedBuffer(const AllocatedBuffer&) = delete;

        AllocatedBuffer(AllocatedBuffer &&src) noexcept
            : Buffer { static_cast<Buffer>(src) }
            , allocator { src.allocator }
            , allocation { std::exchange(src.allocation, nullptr) } { }

        AllocatedBuffer& operator=(const AllocatedBuffer&) = delete;

        AllocatedBuffer& operator=(AllocatedBuffer &&src) noexcept {
            if (allocation) {
                allocator.destroyBuffer(buffer, allocation);
            }

            static_cast<Buffer &>(*this) = static_cast<Buffer>(src);
            allocator = src.allocator;
            buffer = std::exchange(src.buffer, nullptr);
            allocation = std::exchange(src.allocation, nullptr);
            return *this;
        }

        virtual ~AllocatedBuffer() {
            if (allocation) {
                allocator.destroyBuffer(buffer, allocation);
            }
        }
    };
}