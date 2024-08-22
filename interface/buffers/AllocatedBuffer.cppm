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
    export struct AllocatedBuffer : Buffer {
        VMA_HPP_NAMESPACE::Allocator allocator;
        VMA_HPP_NAMESPACE::Allocation allocation;

        AllocatedBuffer(
            VMA_HPP_NAMESPACE::Allocator allocator,
            const VULKAN_HPP_NAMESPACE::BufferCreateInfo &createInfo,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = { {}, VMA_HPP_NAMESPACE::MemoryUsage::eAutoPreferDevice });
        AllocatedBuffer(const AllocatedBuffer&) = delete;
        AllocatedBuffer(AllocatedBuffer &&src) noexcept;
        auto operator=(const AllocatedBuffer&) -> AllocatedBuffer& = delete;
        auto operator=(AllocatedBuffer &&src) noexcept -> AllocatedBuffer&;
        virtual ~AllocatedBuffer();
    };
}

// --------------------
// Implementations.
// --------------------

vku::AllocatedBuffer::AllocatedBuffer(
    VMA_HPP_NAMESPACE::Allocator allocator,
    const VULKAN_HPP_NAMESPACE::BufferCreateInfo &createInfo,
    const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
) : Buffer { nullptr, createInfo.size },
    allocator { allocator } {
    std::tie(buffer, allocation) = allocator.createBuffer(createInfo, allocationCreateInfo);
}

vku::AllocatedBuffer::AllocatedBuffer(
    AllocatedBuffer &&src
) noexcept : Buffer { static_cast<Buffer>(src) },
             allocator { src.allocator },
             allocation { std::exchange(src.allocation, nullptr) } { }

auto vku::AllocatedBuffer::operator=(
    AllocatedBuffer &&src
) noexcept -> AllocatedBuffer & {
    if (allocation) {
        allocator.destroyBuffer(buffer, allocation);
    }

    static_cast<Buffer &>(*this) = static_cast<Buffer>(src);
    allocator = src.allocator;
    buffer = std::exchange(src.buffer, nullptr);
    allocation = std::exchange(src.allocation, nullptr);
    return *this;
}

vku::AllocatedBuffer::~AllocatedBuffer() {
    if (allocation) {
        allocator.destroyBuffer(buffer, allocation);
    }
}