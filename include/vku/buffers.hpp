#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>

#include "details/macros.hpp"

namespace vku {
    struct Buffer {
        vk::Buffer buffer;
        vk::DeviceSize size;

        constexpr operator vk::Buffer() const noexcept {
            return buffer;
        }
    };

    struct AllocatedBuffer : Buffer {
        vma::Allocator allocator;
        vma::Allocation allocation;

        AllocatedBuffer(
            vma::Allocator allocator,
            const vk::BufferCreateInfo &createInfo,
            const vma::AllocationCreateInfo &allocationCreateInfo
        );
        AllocatedBuffer(const AllocatedBuffer&) = delete;
        AllocatedBuffer(AllocatedBuffer &&src) noexcept;
        auto operator=(const AllocatedBuffer&) -> AllocatedBuffer& = delete;
        auto operator=(AllocatedBuffer &&src) noexcept -> AllocatedBuffer&;
        ~AllocatedBuffer();
    };

    struct MappedBuffer : AllocatedBuffer {
        void *data;

        MappedBuffer(AllocatedBuffer buffer);
        template <typename T> requires
            (!std::same_as<T, std::from_range_t>)
            && std::is_standard_layout_v<T>
        MappedBuffer(
            vma::Allocator allocator,
            const T &value,
            vk::BufferUsageFlags usage,
            const vma::AllocationCreateInfo &allocationCreateInfo = {
                vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped,
                vma::MemoryUsage::eAuto,
            }
        ) : MappedBuffer { AllocatedBuffer { allocator, vk::BufferCreateInfo {
                {},
                sizeof(T),
                usage,
            }, allocationCreateInfo } } {
            *static_cast<T*>(data) = value;
        }
        template <typename R> requires std::ranges::input_range<R> && std::ranges::sized_range<R>
        MappedBuffer(
            vma::Allocator allocator,
            std::from_range_t,
            R &&r,
            vk::BufferUsageFlags usage,
            const vma::AllocationCreateInfo &allocationCreateInfo = {
                vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped,
                vma::MemoryUsage::eAuto,
            }
        ) : MappedBuffer { AllocatedBuffer { allocator, vk::BufferCreateInfo {
                {},
                r.size() * sizeof(std::ranges::range_value_t<R>),
                usage,
            }, allocationCreateInfo } } {
            std::ranges::copy(VKU_FWD(r), static_cast<std::ranges::range_value_t<R>*>(data));
        }
        MappedBuffer(const MappedBuffer&) = delete;
        MappedBuffer(MappedBuffer &&src) noexcept = default;
        auto operator=(const MappedBuffer&) -> MappedBuffer& = delete;
        auto operator=(MappedBuffer &&src) noexcept -> MappedBuffer&;
        ~MappedBuffer();

        template <typename T>
        [[nodiscard]] auto asRange(vk::DeviceSize byteOffset = 0) const -> std::span<const T> {
            assert(byteOffset <= size && "Out of bound: byteOffset > size");
            return { reinterpret_cast<const T*>(static_cast<const char*>(data) + byteOffset), (size - byteOffset) / sizeof(T) };
        }

        template <typename T>
        [[nodiscard]] auto asRange(vk::DeviceSize byteOffset = 0) -> std::span<T> {
            assert(byteOffset <= size && "Out of bound: byteOffset > size");
            return { reinterpret_cast<T*>(static_cast<char*>(data) + byteOffset), (size - byteOffset) / sizeof(T) };
        }

        template <typename T>
        [[nodiscard]] auto asValue(vk::DeviceSize byteOffset = 0) const -> const T& {
            assert(byteOffset + sizeof(T) <= size && "Out of bound: byteOffset + sizeof(T) > size");
            return *reinterpret_cast<const T*>(static_cast<const char*>(data) + byteOffset);
        }

        template <typename T>
        [[nodiscard]] auto asValue(vk::DeviceSize byteOffset = 0) -> T& {
            assert(byteOffset + sizeof(T) <= size && "Out of bound: byteOffset + sizeof(T) > size");
            return *reinterpret_cast<T*>(static_cast<char*>(data) + byteOffset);
        }
    };
}