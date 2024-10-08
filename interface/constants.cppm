/** @file constants.cppm
 */

module;

#if !defined(VKU_USE_STD_MODULE) && defined(_MSC_VER)
#include <compare>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:constants;

#if defined(VKU_USE_STD_MODULE) && defined(_MSC_VER)
import std;
#endif

// #define VMA_HPP_NAMESPACE to vma, if not defined.
#ifndef VMA_HPP_NAMESPACE
#define VMA_HPP_NAMESPACE vma
#endif

export import vulkan_hpp;
export import vk_mem_alloc_hpp;

namespace vku::allocation {
    export constexpr VMA_HPP_NAMESPACE::AllocationCreateInfo hostWrite {
        VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessSequentialWrite | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eMapped,
        VMA_HPP_NAMESPACE::MemoryUsage::eAuto,
    };

    export constexpr VMA_HPP_NAMESPACE::AllocationCreateInfo hostRead {
        VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessRandom | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eMapped,
        VMA_HPP_NAMESPACE::MemoryUsage::eAuto,
    };

    export constexpr VMA_HPP_NAMESPACE::AllocationCreateInfo deviceLocal {
        {},
        VMA_HPP_NAMESPACE::MemoryUsage::eAutoPreferDevice,
    };

    export constexpr VMA_HPP_NAMESPACE::AllocationCreateInfo deviceLocalDedicated {
        VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eDedicatedMemory,
        VMA_HPP_NAMESPACE::MemoryUsage::eAutoPreferDevice,
    };

    export constexpr VMA_HPP_NAMESPACE::AllocationCreateInfo deviceLocalTransient {
        {},
        VMA_HPP_NAMESPACE::MemoryUsage::eAutoPreferDevice,
        {}, VULKAN_HPP_NAMESPACE::MemoryPropertyFlagBits::eLazilyAllocated
    };

    export constexpr VMA_HPP_NAMESPACE::AllocationCreateInfo deviceLocalDedicatedTransient {
        VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eDedicatedMemory,
        VMA_HPP_NAMESPACE::MemoryUsage::eAutoPreferDevice,
        {}, VULKAN_HPP_NAMESPACE::MemoryPropertyFlagBits::eLazilyAllocated
    };
}