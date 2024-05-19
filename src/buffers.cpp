#include <vku/buffers.hpp>

vku::AllocatedBuffer::AllocatedBuffer(
    vma::Allocator _allocator,
    const vk::BufferCreateInfo &createInfo,
    const vma::AllocationCreateInfo &allocationCreateInfo
) : Buffer { nullptr, createInfo.size },
    allocator { _allocator } {
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

vku::MappedBuffer::MappedBuffer(
    AllocatedBuffer buffer
) : AllocatedBuffer { std::move(buffer) }, data { allocator.mapMemory(allocation) } { }

auto vku::MappedBuffer::operator=(
    MappedBuffer &&src
) noexcept -> MappedBuffer & {
    if (allocation) {
        allocator.unmapMemory(allocation);
    }
    static_cast<AllocatedBuffer &>(*this) = std::move(static_cast<AllocatedBuffer &>(src));
    data = src.data;
    return *this;
}

vku::MappedBuffer::~MappedBuffer() {
    if (allocation) {
        allocator.unmapMemory(allocation);
    }
}
