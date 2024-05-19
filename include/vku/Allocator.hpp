#pragma once

#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>

namespace vku {
    struct Allocator : vma::Allocator {
        Allocator(
            const vma::AllocatorCreateInfo &createInfo
        ) : vma::Allocator { createAllocator(createInfo) } { }
        Allocator(const Allocator&) = delete;
        Allocator(Allocator &&src) = delete;
        ~Allocator() {
            destroy();
        }
    };
}