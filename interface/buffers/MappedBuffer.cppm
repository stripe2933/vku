module;

#include <cassert>
#ifndef VKU_USE_STD_MODULE
#include <algorithm>
#include <concepts>
#include <ranges>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#endif

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

namespace vku {
    export struct MappedBuffer : AllocatedBuffer {
        void *data;

        MappedBuffer(
            vma::Allocator allocator,
            const vk::BufferCreateInfo &createInfo,
            const vma::AllocationCreateInfo &allocationCreateInfo = {
                vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped,
                vma::MemoryUsage::eAuto,
            });
        template <typename T> requires (!std::same_as<T, std::from_range_t> && std::is_standard_layout_v<T>)
        MappedBuffer(
            vma::Allocator allocator,
            const T &value,
            vk::BufferUsageFlags usage,
            const vma::AllocationCreateInfo &allocationCreateInfo = {
                vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped,
                vma::MemoryUsage::eAuto,
            });
        template <std::ranges::input_range R> requires (std::ranges::sized_range<R> && std::is_standard_layout_v<std::ranges::range_value_t<R>>)
        MappedBuffer(
            vma::Allocator allocator,
            std::from_range_t,
            R &&r,
            vk::BufferUsageFlags usage,
            const vma::AllocationCreateInfo &allocationCreateInfo = {
                vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped,
                vma::MemoryUsage::eAuto,
            });
        MappedBuffer(const MappedBuffer&) = delete;
        MappedBuffer(MappedBuffer &&src) noexcept = default;
        auto operator=(const MappedBuffer&) -> MappedBuffer& = delete;
        auto operator=(MappedBuffer &&src) noexcept -> MappedBuffer&;
        ~MappedBuffer();

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
        [[nodiscard]] auto asRange(vk::DeviceSize byteOffset = 0) const NOEXCEPT_IF_RELEASE -> std::span<const T>;

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
        [[nodiscard]] auto asRange(vk::DeviceSize byteOffset = 0) NOEXCEPT_IF_RELEASE -> std::span<T>;

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
    };
}

// --------------------
// Implementations.
// --------------------

vku::MappedBuffer::MappedBuffer(
    vma::Allocator allocator,
    const vk::BufferCreateInfo &createInfo,
    const vma::AllocationCreateInfo &allocationCreateInfo
) : AllocatedBuffer { allocator, createInfo, allocationCreateInfo },
    data { allocator.mapMemory(allocation) } { }

template <typename T> requires (!std::same_as<T, std::from_range_t> && std::is_standard_layout_v<T>)
vku::MappedBuffer::MappedBuffer(
    vma::Allocator allocator,
    const T &value,
    vk::BufferUsageFlags usage,
    const vma::AllocationCreateInfo &allocationCreateInfo
) : MappedBuffer { allocator, vk::BufferCreateInfo {
        {},
        sizeof(T),
        usage,
    }, allocationCreateInfo } {
    *static_cast<T*>(data) = value;
}

template <std::ranges::input_range R> requires (std::ranges::sized_range<R> && std::is_standard_layout_v<std::ranges::range_value_t<R>>)
vku::MappedBuffer::MappedBuffer(
    vma::Allocator allocator,
    std::from_range_t,
    R &&r,
    vk::BufferUsageFlags usage,
    const vma::AllocationCreateInfo &allocationCreateInfo
) : MappedBuffer { allocator, vk::BufferCreateInfo {
        {},
        r.size() * sizeof(std::ranges::range_value_t<R>),
        usage,
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

vku::MappedBuffer::~MappedBuffer() {
    if (allocation) {
        allocator.unmapMemory(allocation);
    }
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