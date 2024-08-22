module;

#include <cassert>
#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <algorithm>
#include <concepts>
#include <ranges>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:buffers.MappedBuffer;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import :buffers.AllocatedBuffer;

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)
#ifdef NDEBUG
#define NOEXCEPT_IF_RELEASE noexcept
#else
#define NOEXCEPT_IF_RELEASE
#endif

// #define VMA_HPP_NAMESPACE to vma, if not defined.
#ifndef VMA_HPP_NAMESPACE
#define VMA_HPP_NAMESPACE vma
#endif

namespace vku {
    export struct MappedBuffer : AllocatedBuffer {
        void *data;

        MappedBuffer(
            VMA_HPP_NAMESPACE::Allocator allocator,
            const VULKAN_HPP_NAMESPACE::BufferCreateInfo &createInfo,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = {
                VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessSequentialWrite | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eMapped,
                VMA_HPP_NAMESPACE::MemoryUsage::eAuto,
            });
        template <typename T> requires (!std::same_as<T, std::from_range_t> && std::is_standard_layout_v<T>)
        MappedBuffer(
            VMA_HPP_NAMESPACE::Allocator allocator,
            const T &value,
            VULKAN_HPP_NAMESPACE::BufferUsageFlags usage,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = {
                VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessSequentialWrite | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eMapped,
                VMA_HPP_NAMESPACE::MemoryUsage::eAuto,
            });
        template <typename T> requires (!std::same_as<T, std::from_range_t> && std::is_standard_layout_v<T>)
        MappedBuffer(
            VMA_HPP_NAMESPACE::Allocator allocator,
            const T &value,
            VULKAN_HPP_NAMESPACE::BufferUsageFlags usage,
            VULKAN_HPP_NAMESPACE::ArrayProxy<const std::uint32_t> queueFamilyIndices,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = {
                VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessSequentialWrite | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eMapped,
                VMA_HPP_NAMESPACE::MemoryUsage::eAuto,
            });
        template <std::ranges::input_range R> requires (std::ranges::sized_range<R> && std::is_standard_layout_v<std::ranges::range_value_t<R>>)
        MappedBuffer(
            VMA_HPP_NAMESPACE::Allocator allocator,
            std::from_range_t, R &&r,
            VULKAN_HPP_NAMESPACE::BufferUsageFlags usage,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = {
                VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessSequentialWrite | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eMapped,
                VMA_HPP_NAMESPACE::MemoryUsage::eAuto,
            });
        template <std::ranges::input_range R> requires (std::ranges::sized_range<R> && std::is_standard_layout_v<std::ranges::range_value_t<R>>)
        MappedBuffer(
            VMA_HPP_NAMESPACE::Allocator allocator,
            std::from_range_t, R &&r,
            VULKAN_HPP_NAMESPACE::BufferUsageFlags usage,
            VULKAN_HPP_NAMESPACE::ArrayProxy<const std::uint32_t> queueFamilyIndices,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = {
                VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessSequentialWrite | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eMapped,
                VMA_HPP_NAMESPACE::MemoryUsage::eAuto,
            });
        MappedBuffer(const MappedBuffer&) = delete;
        MappedBuffer(MappedBuffer &&src) noexcept = default;
        auto operator=(const MappedBuffer&) -> MappedBuffer& = delete;
        auto operator=(MappedBuffer &&src) noexcept -> MappedBuffer&;
        ~MappedBuffer() override {
            if (allocation) {
                allocator.unmapMemory(allocation);
            }
        }

        /**
         * Get <tt>std::span<const T></tt> from the mapped memory range.
         * @tparam T Type of the elements in the span.
         * @param byteOffset Begin offset in bytes.
         * @return <tt>std::span<const T></tt>, whose address starts from (mapped address) + \p byteOffset.
         * @note
         * It internally asserts \p byteOffset is less than or equal to the buffer size for debug (NDEBUG is not
         * defined) environment.
         */
        template <typename T>
        [[nodiscard]] auto asRange(VULKAN_HPP_NAMESPACE::DeviceSize byteOffset = 0) const NOEXCEPT_IF_RELEASE -> std::span<const T>;

        /**
         * Get <tt>std::span<T></tt> from the mapped memory range.
         * @tparam T Type of the elements in the span.
         * @param byteOffset Begin offset in bytes.
         * @return <tt>std::span<T></tt>, whose address starts from (mapped address) + \p byteOffset.
         * @note
         * It internally asserts \p byteOffset is less than or equal to the buffer size for debug (NDEBUG is not
         * defined) environment.
         */
        template <typename T>
        [[nodiscard]] auto asRange(VULKAN_HPP_NAMESPACE::DeviceSize byteOffset = 0) NOEXCEPT_IF_RELEASE -> std::span<T>;

        /**
         * Get const reference of <tt>T</tt> from the mapped memory.
         * @tparam T Type of the reference.
         * @param byteOffset Begin offset in bytes.
         * @return Const reference of <tt>T</tt>, whose address starts from (mapped address) + \p byteOffset.
         * @note
         * It internally asserts <tt>byteOffset + sizeof(T)</tt> is less than or equal to the buffer size for debug
         * (NDEBUG is not defined) environment.
         */
        template <typename T>
        [[nodiscard]] auto asValue(vk::DeviceSize byteOffset = 0) const NOEXCEPT_IF_RELEASE -> const T&;

        /**
         * Get const reference of <tt>T</tt> from the mapped memory.
         * @tparam T Type of the reference.
         * @param byteOffset Begin offset in bytes.
         * @return Reference of <tt>T</tt>, whose address starts from (mapped address) + \p byteOffset.
         * @note
         * It internally asserts <tt>byteOffset + sizeof(T)</tt> is less than or equal to the buffer size for debug
         * (NDEBUG is not defined) environment.
         */
        template <typename T>
        [[nodiscard]] auto asValue(vk::DeviceSize byteOffset = 0) NOEXCEPT_IF_RELEASE -> T&;

        [[nodiscard]] auto unmap() && noexcept -> AllocatedBuffer {
            allocator.unmapMemory(allocation);
            return static_cast<AllocatedBuffer>(std::move(*this));
        }
    };
}

// --------------------
// Implementations.
// --------------------

vku::MappedBuffer::MappedBuffer(
    VMA_HPP_NAMESPACE::Allocator allocator,
    const vk::BufferCreateInfo &createInfo,
    const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
) : AllocatedBuffer { allocator, createInfo, allocationCreateInfo },
    data { allocator.mapMemory(allocation) } { }

template <typename T> requires (!std::same_as<T, std::from_range_t> && std::is_standard_layout_v<T>)
vku::MappedBuffer::MappedBuffer(
    VMA_HPP_NAMESPACE::Allocator allocator,
    const T &value,
    vk::BufferUsageFlags usage,
    const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
) : MappedBuffer { allocator, vk::BufferCreateInfo {
        {},
        sizeof(T),
        usage,
    }, allocationCreateInfo } {
    *static_cast<T*>(data) = value;
}

template <typename T> requires (!std::same_as<T, std::from_range_t> && std::is_standard_layout_v<T>)
vku::MappedBuffer::MappedBuffer(
    VMA_HPP_NAMESPACE::Allocator allocator,
    const T &value,
    vk::BufferUsageFlags usage,
    vk::ArrayProxy<const std::uint32_t> queueFamilyIndices,
    const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
) : MappedBuffer { allocator, vk::BufferCreateInfo {
        {},
        sizeof(T),
        usage,
        queueFamilyIndices.size() == 1 ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent, queueFamilyIndices,
    }, allocationCreateInfo } {
    *static_cast<T*>(data) = value;
}

template <std::ranges::input_range R> requires (std::ranges::sized_range<R> && std::is_standard_layout_v<std::ranges::range_value_t<R>>)
vku::MappedBuffer::MappedBuffer(
    VMA_HPP_NAMESPACE::Allocator allocator,
    std::from_range_t,
    R &&r,
    vk::BufferUsageFlags usage,
    const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
) : MappedBuffer { allocator, vk::BufferCreateInfo {
        {},
        r.size() * sizeof(std::ranges::range_value_t<R>),
        usage,
    }, allocationCreateInfo } {
    std::ranges::copy(FWD(r), static_cast<std::ranges::range_value_t<R>*>(data));
}

template <std::ranges::input_range R> requires (std::ranges::sized_range<R> && std::is_standard_layout_v<std::ranges::range_value_t<R>>)
vku::MappedBuffer::MappedBuffer(
    vma::Allocator allocator,
    std::from_range_t,
    R &&r,
    vk::BufferUsageFlags usage,
    vk::ArrayProxy<const std::uint32_t> queueFamilyIndices,
    const vma::AllocationCreateInfo &allocationCreateInfo
) : MappedBuffer { allocator, vk::BufferCreateInfo {
        {},
        r.size() * sizeof(std::ranges::range_value_t<R>),
        usage,
        queueFamilyIndices.size() == 1 ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent, queueFamilyIndices,
    }, allocationCreateInfo } {
    std::ranges::copy(FWD(r), static_cast<std::ranges::range_value_t<R>*>(data));
}

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

template <typename T>
auto vku::MappedBuffer::asRange(
    vk::DeviceSize byteOffset
) const NOEXCEPT_IF_RELEASE -> std::span<const T> {
    assert(byteOffset <= size && "Out of bound: byteOffset > size");
    return { reinterpret_cast<const T*>(static_cast<const char*>(data) + byteOffset), (size - byteOffset) / sizeof(T) };
}

template <typename T>
auto vku::MappedBuffer::asRange(
    vk::DeviceSize byteOffset
) NOEXCEPT_IF_RELEASE -> std::span<T> {
    assert(byteOffset <= size && "Out of bound: byteOffset > size");
    return { reinterpret_cast<T*>(static_cast<char*>(data) + byteOffset), (size - byteOffset) / sizeof(T) };
}

template <typename T>
auto vku::MappedBuffer::asValue(
    vk::DeviceSize byteOffset
) const NOEXCEPT_IF_RELEASE -> const T& {
    assert(byteOffset + sizeof(T) <= size && "Out of bound: byteOffset + sizeof(T) > size");
    return *reinterpret_cast<const T*>(static_cast<const char*>(data) + byteOffset);
}

template <typename T>
auto vku::MappedBuffer::asValue(
    vk::DeviceSize byteOffset
) NOEXCEPT_IF_RELEASE -> T& {
    assert(byteOffset + sizeof(T) <= size && "Out of bound: byteOffset + sizeof(T) > size");
    return *reinterpret_cast<T*>(static_cast<char*>(data) + byteOffset);
}