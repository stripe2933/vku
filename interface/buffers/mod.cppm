/** @file buffers/mod.cppm
 */

module;

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:buffers;
export import :buffers.AllocatedBuffer;
export import :buffers.Buffer;
export import :buffers.MappedBuffer;

import std;
import :utils;

// #define VMA_HPP_NAMESPACE to vma, if not defined.
#ifndef VMA_HPP_NAMESPACE
#define VMA_HPP_NAMESPACE vma
#endif

namespace vku {
    /**
     * @brief Try to create a staging destination buffer of both visible to host and device local.
     *
     * In UMA architectures, you don't have to transfer the data from host visible buffer to device local buffer, since
     * they are both likely to be in the same memory heap. This function provides a simple mechanism to create a buffer
     * of this purpose.
     *
     * @param allocator VMA allocator used to allocate memory.
     * @param createInfo Buffer create info.
     * @return Variant of either AllocatedBuffer (non-UMA architecture) or MappedBuffer (UMA architecture).
     */
    export
    [[nodiscard]] std::variant<AllocatedBuffer, MappedBuffer> createStagingDstBuffer(
        VMA_HPP_NAMESPACE::Allocator allocator,
        const VULKAN_HPP_NAMESPACE::BufferCreateInfo &createInfo,
        const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = {
            VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessSequentialWrite | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessAllowTransferInstead | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eMapped,
            VMA_HPP_NAMESPACE::MemoryUsage::eAuto,
        }
    ) {
        AllocatedBuffer buffer { allocator, createInfo, allocationCreateInfo };

        if (contains(allocator.getAllocationMemoryProperties(buffer.allocation), VULKAN_HPP_NAMESPACE::MemoryPropertyFlagBits::eHostVisible)) {
            return MappedBuffer { std::move(buffer) };
        }
        else {
            return { std::move(buffer) };
        }
    }
}