/** @file buffers/MappedBuffer.cppm
 */

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

#include <span>
#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:buffers.MappedBuffer;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import :buffers.AllocatedBuffer;
import :utils;

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
    /**
     * @brief Owning buffer handle with memory allocation and CPU-accessible mapped data.
     */
    export class MappedBuffer : public AllocatedBuffer {
    public:
        /**
         * @brief Start address of the mapped memory.
         *
         * This is only valid during the lifetime of this object. Destroying this object will unmap the memory.
         */
        void *data;

        /**
         * @brief Create mapped buffer.
         * @param allocator VMA allocator used to allocate memory.
         * @param createInfo Buffer create info.
         * @param allocationCreateInfo Allocation create info.
         */
        MappedBuffer(
            VMA_HPP_NAMESPACE::Allocator allocator,
            const VULKAN_HPP_NAMESPACE::BufferCreateInfo &createInfo,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = {
                VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessSequentialWrite | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eMapped,
                VMA_HPP_NAMESPACE::MemoryUsage::eAuto,
            }
        ) : AllocatedBuffer { allocator, createInfo, allocationCreateInfo },
            data { allocator.mapMemory(allocation) } { }

        /**
         * @brief Create mapped buffer of the given \p value and exclusive sharing mode.
         * @tparam T Type of the value, must be trivially copyable.
         * @param allocator VMA allocator used to allocate memory.
         * @param value Value that will be copied to the whole buffer.
         * @param usage Buffer usage flags.
         * @param allocationCreateInfo Allocation create info.
         */
        template <typename T> requires (!std::same_as<T, std::from_range_t> && std::is_trivially_copyable_v<T>)
        MappedBuffer(
            VMA_HPP_NAMESPACE::Allocator allocator,
            const T &value,
            VULKAN_HPP_NAMESPACE::BufferUsageFlags usage,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = {
                VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessSequentialWrite | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eMapped,
                VMA_HPP_NAMESPACE::MemoryUsage::eAuto,
            }
        ) : MappedBuffer { allocator, VULKAN_HPP_NAMESPACE::BufferCreateInfo {
                {},
                sizeof(T),
                usage,
            }, allocationCreateInfo } {
            *static_cast<T*>(data) = value;
        }

        /**
         * @brief Create mapped buffer of the given \p value and concurrent sharing mode.
         * @tparam T Type of the value, must be trivially copyable.
         * @param allocator VMA allocator used to allocate memory.
         * @param value %Value that will be copied to the whole buffer.
         * @param usage %Buffer usage flags.
         * @param queueFamilyIndices %Queue family indices that would be used at the concurrent sharing of the resource.
         * @param allocationCreateInfo %Allocation create info.
         */
        template <typename T> requires (!std::same_as<T, std::from_range_t> && std::is_trivially_copyable_v<T>)
        MappedBuffer(
            VMA_HPP_NAMESPACE::Allocator allocator,
            const T &value,
            VULKAN_HPP_NAMESPACE::BufferUsageFlags usage,
            VULKAN_HPP_NAMESPACE::ArrayProxy<const std::uint32_t> queueFamilyIndices,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = {
                VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessSequentialWrite | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eMapped,
                VMA_HPP_NAMESPACE::MemoryUsage::eAuto,
            }
        ) : MappedBuffer { allocator, VULKAN_HPP_NAMESPACE::BufferCreateInfo {
                {},
                sizeof(T),
                usage,
                queueFamilyIndices.size() == 1 ? VULKAN_HPP_NAMESPACE::SharingMode::eExclusive : VULKAN_HPP_NAMESPACE::SharingMode::eConcurrent, queueFamilyIndices,
            }, allocationCreateInfo } {
            *static_cast<T*>(data) = value;
        }

        /**
         * @brief Create mapped buffer of the given range with exclusive sharing mode.
         * @tparam R Range type. It must be sized and its element type must be trivially copyable.
         * @param allocator VMA allocator used to allocate memory.
         * @param r Range of the elements that will be copied to the buffer.
         * @param usage %Buffer usage flags.
         * @param allocationCreateInfo %Allocation create info.
         */
        template <std::ranges::input_range R> requires (std::ranges::sized_range<R> && std::is_trivially_copyable_v<std::ranges::range_value_t<R>>)
        MappedBuffer(
            VMA_HPP_NAMESPACE::Allocator allocator,
            std::from_range_t, R &&r,
            VULKAN_HPP_NAMESPACE::BufferUsageFlags usage,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = {
                VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessSequentialWrite | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eMapped,
                VMA_HPP_NAMESPACE::MemoryUsage::eAuto,
            }
        ) : MappedBuffer { allocator, VULKAN_HPP_NAMESPACE::BufferCreateInfo {
                {},
                r.size() * sizeof(std::ranges::range_value_t<R>),
                usage,
            }, allocationCreateInfo } {
            std::ranges::copy(FWD(r), static_cast<std::ranges::range_value_t<R>*>(data));
        }

        /**
         * @brief Create mapped buffer of the given range with concurrent sharing mode.
         * @tparam R Range type. It must be sized and its element type must be trivially copyable.
         * @param allocator VMA allocator used to allocate memory.
         * @param r Range of the elements that will be copied to the buffer.
         * @param usage %Buffer usage flags.
         * @param queueFamilyIndices %Queue family indices that would be used at the concurrent sharing of the resource.
         * @param allocationCreateInfo %Allocation create info.
         */
        template <std::ranges::input_range R> requires (std::ranges::sized_range<R> && std::is_trivially_copyable_v<std::ranges::range_value_t<R>>)
        MappedBuffer(
            VMA_HPP_NAMESPACE::Allocator allocator,
            std::from_range_t, R &&r,
            VULKAN_HPP_NAMESPACE::BufferUsageFlags usage,
            VULKAN_HPP_NAMESPACE::ArrayProxy<const std::uint32_t> queueFamilyIndices,
            const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo = {
                VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessSequentialWrite | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eMapped,
                VMA_HPP_NAMESPACE::MemoryUsage::eAuto,
            }
        ) : MappedBuffer { allocator, VULKAN_HPP_NAMESPACE::BufferCreateInfo {
                {},
                r.size() * sizeof(std::ranges::range_value_t<R>),
                usage,
                queueFamilyIndices.size() == 1 ? VULKAN_HPP_NAMESPACE::SharingMode::eExclusive : VULKAN_HPP_NAMESPACE::SharingMode::eConcurrent, queueFamilyIndices,
            }, allocationCreateInfo } {
            std::ranges::copy(FWD(r), static_cast<std::ranges::range_value_t<R>*>(data));
        }

        MappedBuffer(const MappedBuffer&) = delete;

        MappedBuffer(MappedBuffer &&src) noexcept = default;

        MappedBuffer& operator=(const MappedBuffer&) = delete;

        MappedBuffer& operator=(MappedBuffer &&src) noexcept {
            if (allocation) {
                allocator.unmapMemory(allocation);
            }
            static_cast<AllocatedBuffer &>(*this) = std::move(static_cast<AllocatedBuffer &>(src));
            data = src.data;
            return *this;
        }

        ~MappedBuffer() override {
            if (allocation) {
                allocator.unmapMemory(allocation);
            }
        }

        /**
         * @brief Get <tt>std::span<const T></tt> from the mapped memory range.
         * @tparam T Type of the elements in the span.
         * @param byteOffset Beginning offset in bytes. The remaining size must be greater than or equal to <tt>sizeof(T)</tt>.
         * @return <tt>std::span<const T></tt>, whose address starts from (mapped address) + \p byteOffset.
         */
        template <typename T>
        [[nodiscard]] std::span<const T> asRange(VULKAN_HPP_NAMESPACE::DeviceSize byteOffset = 0) const NOEXCEPT_IF_RELEASE {
            assert(byteOffset <= size && "Out of bound: byteOffset > size");
            return { reinterpret_cast<const T*>(static_cast<const char*>(data) + byteOffset), (size - byteOffset) / sizeof(T) };
        }

        /**
         * @brief Get <tt>std::span<T></tt> from the mapped memory range.
         * @tparam T Type of the elements in the span.
         * @param byteOffset Beginning offset in bytes. The remaining size must be greater than or equal to <tt>sizeof(T)</tt>.
         * @return <tt>std::span<T></tt>, whose address starts from (mapped address) + \p byteOffset.
         */
        template <typename T>
        [[nodiscard]] std::span<T> asRange(VULKAN_HPP_NAMESPACE::DeviceSize byteOffset = 0) NOEXCEPT_IF_RELEASE {
            assert(byteOffset <= size && "Out of bound: byteOffset > size");
            return { reinterpret_cast<T*>(static_cast<char*>(data) + byteOffset), (size - byteOffset) / sizeof(T) };
        }

        /**
         * @brief Get const reference of <tt>T</tt> from the mapped memory.
         * @tparam T Type of the reference.
         * @param byteOffset Beginning offset in bytes. The remaining size must be greater than or equal to <tt>sizeof(T)</tt>.
         * @return Const reference of <tt>T</tt>, whose address starts from (mapped address) + \p byteOffset.
         */
        template <typename T>
        [[nodiscard]] const T& asValue(VULKAN_HPP_NAMESPACE::DeviceSize byteOffset = 0) const NOEXCEPT_IF_RELEASE {
            assert(byteOffset + sizeof(T) <= size && "Out of bound: byteOffset + sizeof(T) > size");
            return *reinterpret_cast<const T*>(static_cast<const char*>(data) + byteOffset);
        }

        /**
         * @brief Get const reference of <tt>T</tt> from the mapped memory.
         * @tparam T Type of the reference.
         * @param byteOffset Beginning offset in bytes. The remaining size must be greater than or equal to <tt>sizeof(T)</tt>.
         * @return Reference of <tt>T</tt>, whose address starts from (mapped address) + \p byteOffset.
         */
        template <typename T>
        [[nodiscard]] T& asValue(VULKAN_HPP_NAMESPACE::DeviceSize byteOffset = 0) NOEXCEPT_IF_RELEASE {
            assert(byteOffset + sizeof(T) <= size && "Out of bound: byteOffset + sizeof(T) > size");
            return *reinterpret_cast<T*>(static_cast<char*>(data) + byteOffset);
        }

        /**
         * @brief Unmap the allocation and slice this object to <tt>AllocatedBuffer</tt>.
         * @return Slice of this object.
         */
        [[nodiscard]] AllocatedBuffer unmap() && noexcept {
            allocator.unmapMemory(allocation);
            return static_cast<AllocatedBuffer>(std::move(*this));
        }

    private:
        explicit MappedBuffer(AllocatedBuffer &&allocatedBuffer)
            : AllocatedBuffer { std::move(allocatedBuffer) }
            , data { allocator.mapMemory(allocation) } { }

        friend std::variant<AllocatedBuffer, MappedBuffer> createStagingDstBuffer(VMA_HPP_NAMESPACE::Allocator, const VULKAN_HPP_NAMESPACE::BufferCreateInfo&, const VMA_HPP_NAMESPACE::AllocationCreateInfo&);
    };
}