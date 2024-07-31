export module vku:constants;

export import vulkan_hpp;
export import vk_mem_alloc_hpp;

namespace vku::allocation {
    export constexpr vma::AllocationCreateInfo hostWrite {
        vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped,
        vma::MemoryUsage::eAuto,
    };

    export constexpr vma::AllocationCreateInfo hostRead {
        vma::AllocationCreateFlagBits::eHostAccessRandom | vma::AllocationCreateFlagBits::eMapped,
        vma::MemoryUsage::eAuto,
    };

    export constexpr vma::AllocationCreateInfo deviceLocal {
        {},
        vma::MemoryUsage::eAutoPreferDevice,
    };

    export constexpr vma::AllocationCreateInfo deviceLocalDedicated {
        vma::AllocationCreateFlagBits::eDedicatedMemory,
        vma::MemoryUsage::eAutoPreferDevice,
    };

    export constexpr vma::AllocationCreateInfo deviceLocalTransient {
        {},
        vma::MemoryUsage::eAutoPreferDevice,
        {}, vk::MemoryPropertyFlagBits::eLazilyAllocated
    };

    export constexpr vma::AllocationCreateInfo deviceLocalDedicatedTransient {
        vma::AllocationCreateFlagBits::eDedicatedMemory,
        vma::MemoryUsage::eAutoPreferDevice,
        {}, vk::MemoryPropertyFlagBits::eLazilyAllocated
    };
}