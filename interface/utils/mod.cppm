/** @file utils/mod.cppm
 */

module;

#include <cassert>
#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <algorithm>
#include <concepts>
#include <functional>
#include <initializer_list>
#include <ranges>
#include <utility>
#include <vector>
#ifdef _MSC_VER
#include <compare>
#endif
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:utils;
export import :utils.RefHolder;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vulkan_hpp;

#ifdef NDEBUG
#define NOEXCEPT_IF_RELEASE noexcept
#else
#define NOEXCEPT_IF_RELEASE
#endif
#define FWD(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__)

namespace vku {
    /**
     * @brief Take a temporary value and return its address.
     *
     * In Vulkan-Hpp API, you can use this function to pass temporary values to functions that require a pointer.
     * For example, the following code creating a Vulkan instance with an application info
     * @code{.cpp}
     * vk::raii::Instance instance { context, vk::InstanceCreateInfo {
     *     {},
     *     &vk::ApplicationInfo { ... }, // ERROR! Cannot take the address of a temporary value.
     * } }; // But vk::ApplicationInfo destroyed here.
     * @endcode
     * is invalid, but you know that the <tt>vk::ApplicationInfo</tt> struct would remain its lifetime until the
     * end of the expression. Although you can declare the struct before the instance creation line, it's quite verbose
     * and you'll get tired for naming such trivial stuffs.
     *
     * Instead, you can use this function to pass the temporary value to the function:
     * @code{.cpp}
     * vk::raii::Instance instance { context, vk::InstanceCreateInfo {
     *     {},
     *     vku::unsafeAddress(vk::ApplicationInfo { ... }),
     * } }; // vk::ApplicationInfo destroyed here, however vk::raii::Instance already created and it's fine.
     * @endcode
     *
     * @param value Temporary value.
     * @return Address of the temporary value.
     * @warning The result address is only valid until the end of the expression, which contains the temporary value.
     */
    export template <typename T>
    [[nodiscard]] const T* unsafeAddress(const T &value [[clang::lifetimebound]]) noexcept {
        return &value;
    }

    /**
     * @brief Take a temporary value and return <tt>vk::ArrayProxyNoTemporaries</tt> of it.
     *
     * @code{.cpp}
     * vk::raii::Device device { physicalDevice, vk::DeviceCreateInfo {
     *     {},
     *     // { vk::DeviceQueueCreateInfo { ... } }, // ERROR! calling deleted constructor of vk::ArrayProxyNoTemporaries<const vk::DeviceQueueCreateInfo>.
     *     vku::unsafeProxy(vk::DeviceQueueCreateInfo { ... }), // OK. Just make ensure that vk::DeviceQueueCreateInfo make alive until the end of the device creation.
     *     ...
     * };
     * @endcode
     *
     * @param value Temporary value.
     * @return <tt>vk::ArrayProxyNoTemporaries</tt> of the temporary value.
     * @note The intention of this function follows the same reason as <tt>unsafeAddress</tt> function.
     * @warning The result is only valid until the end of the expression, which contains the temporary value.
     */
    export template <typename T>
    [[nodiscard]] VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const T> unsafeProxy(const T &value [[clang::lifetimebound]]) noexcept {
        return value;
    }

    /**
     * @brief Take a temporary contiguous range and return <tt>vk::ArrayProxyNoTemporaries</tt> of it.
     * @param range Temporary contiguous range.
     * @return <tt>vk::ArrayProxyNoTemporaries</tt> of the temporary range.
     * @note The intention of this function follows the same reason as <tt>unsafeAddress</tt> function.
     * @warning The result is only valid until the end of the expression, which contains the temporary range.
     */
    export template <std::ranges::contiguous_range R>
    [[nodiscard]] VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const std::ranges::range_value_t<R>> unsafeProxy(const R &range [[clang::lifetimebound]]) noexcept {
        return range;
    }

    /**
     * @brief Take a temporary initializer list and return <tt>vk::ArrayProxyNoTemporaries</tt> of it.
     *
     * @code{.cpp}
     * vk::raii::DescriptorSetLayout device { device, vk::DescriptorSetLayoutCreateInfo {
     *     {},
     *     // { vk::DescriptorSetLayoutBinding { ... }, ... }, // ERROR! calling deleted constructor of vk::ArrayProxyNoTemporaries<const vk::DescriptorSetLayoutBinding>.
     *     vku::unsafeProxy({
     *         vk::DescriptorSetLayoutBinding { ... },
     *         vk::DescriptorSetLayoutBinding { ... },
     *         ...
     *     }), // OK. Just make ensure that vk::DescriptorSetLayoutBindings alive until the end of the descriptor set layout creation.
     * };
     * @endcode
     *
     * @param list Temporary initializer list.
     * @return <tt>vk::ArrayProxyNoTemporaries</tt> of the temporary initializer list.
     * @note The intention of this function follows the same reason as <tt>unsafeAddress</tt> function.
     * @warning The result is only valid until the end of the expression, which contains the temporary initializer list.
     */
    export template <typename T>
    [[nodiscard]] VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const T> unsafeProxy(const std::initializer_list<T> &list [[clang::lifetimebound]]) noexcept {
        return list;
    }

    /**
     * @brief Check whether \p flag is contained in \p flags.
     * @param flags Vulkan flags.
     * @param flag Vulkan flag bit.
     * @return <tt>true</tt> If flag is contained in flags, <tt>false</tt> otherwise.
     * @see contains(vk::Flags<T>, vk::Flags<T>) -> bool
     */
    export template <typename T>
    [[nodiscard]] constexpr bool contains(VULKAN_HPP_NAMESPACE::Flags<T> flags, T flag) noexcept {
        return (flags & flag) == flag;
    }

    /**
     * @brief Check whether \p sub is contained in \p super.
     * @param super Vulkan flags.
     * @param sub Vulkan flags.
     * @return <tt>true</tt> If sub is contained in super, <tt>false</tt> otherwise.
     * @see contains(vk::Flags<T>, T) -> bool
     */
    export template <typename T>
    [[nodiscard]] constexpr bool contains(VULKAN_HPP_NAMESPACE::Flags<T> super, VULKAN_HPP_NAMESPACE::Flags<T> sub) noexcept {
        return (super & sub) == sub;
    }

    /**
     * @brief Convert <tt>vk::Extent3D</tt> to <tt>vk::Extent2D</tt>.
     *
     * The depth component is discarded.
     *
     * @param extent Extent to convert.
     * @return Converted extent.
     */
    export
    [[nodiscard]] constexpr VULKAN_HPP_NAMESPACE::Extent2D toExtent2D(const VULKAN_HPP_NAMESPACE::Extent3D &extent) noexcept {
        return { extent.width, extent.height };
    }

    /**
     * @brief Convert <tt>vk::Offset2D</tt> to <tt>vk::Extent2D</tt>.
     *
     * Negative component is converted to the least unsigned integer congruent to the source integer.
     *
     * @param offset Offset to convert.
     * @return Converted extent.
     * @note Negative component will be cast to unsigned int, with C++ standard conversion rule.
     */
    export
    [[nodiscard]] constexpr VULKAN_HPP_NAMESPACE::Extent2D toExtent2D(const VULKAN_HPP_NAMESPACE::Offset2D &offset) noexcept {
        return { static_cast<std::uint32_t>(offset.x), static_cast<std::uint32_t>(offset.y) };
    }

    /**
     * @brief Convert <tt>vk::Offset3D</tt> to <tt>vk::Extent3D</tt>.
     *
     * Negative component is converted to the least unsigned integer congruent to the source integer.
     *
     * @param offset Offset to convert.
     * @return Converted extent.
     * @note Negative component will be cast to unsigned int, with C++ standard conversion rule.
     */
    export
    [[nodiscard]] constexpr VULKAN_HPP_NAMESPACE::Extent3D toExtent3D(const VULKAN_HPP_NAMESPACE::Offset3D &offset) noexcept {
        return { static_cast<std::uint32_t>(offset.x), static_cast<std::uint32_t>(offset.y), static_cast<std::uint32_t>(offset.z) };
    }

    /**
     * @brief Convert <tt>vk::Offset3D</tt> to <tt>vk::Offset2D</tt>.
     *
     * The z component is discarded.
     *
     * @param offset Offset to convert.
     * @return Converted offset.
     */
    export
    [[nodiscard]] constexpr VULKAN_HPP_NAMESPACE::Offset2D toOffset2D(const VULKAN_HPP_NAMESPACE::Offset3D &offset) noexcept {
        return { offset.x, offset.y };
    }

    /**
     * @brief Convert <tt>vk::Extent2D</tt> to <tt>vk::Offset2D</tt>.
     * @param extent Extent to convert. Its width and height must be less than or equal to <tt>2^31 - 1</tt>.
     * @return Converted offset.
     */
    export
    [[nodiscard]] constexpr VULKAN_HPP_NAMESPACE::Offset2D toOffset2D(const VULKAN_HPP_NAMESPACE::Extent2D &extent) NOEXCEPT_IF_RELEASE {
        assert(std::in_range<std::int32_t>(extent.width) && "Overflowing width.");
        assert(std::in_range<std::int32_t>(extent.height) && "Overflowing height.");
        return { static_cast<std::int32_t>(extent.width), static_cast<std::int32_t>(extent.height) };
    }

    /**
     * @brief Convert <tt>vk::Extent3D</tt> to <tt>vk::Offset3D</tt>.
     * @param extent Extent to convert. Its width, height and depth must be less than or equal to <tt>2^31 - 1</tt>.
     * @return Converted offset.
     */
    export
    [[nodiscard]] constexpr VULKAN_HPP_NAMESPACE::Offset3D toOffset3D(const VULKAN_HPP_NAMESPACE::Extent3D &extent) NOEXCEPT_IF_RELEASE {
        assert(std::in_range<std::int32_t>(extent.width) && "Overflowing width.");
        assert(std::in_range<std::int32_t>(extent.height) && "Overflowing height.");
        assert(std::in_range<std::int32_t>(extent.depth) && "Overflowing depth.");
        return { static_cast<std::int32_t>(extent.width), static_cast<std::int32_t>(extent.height), static_cast<std::int32_t>(extent.depth) };
    }

    /**
     * @brief Convert <tt>vk::Extent2D</tt> to <tt>vk::Viewport</tt>, with depth=0..1.
     *
     * For example, setting viewport and scissor as full region of the swapchain for pipeline dynamic state would be:
     * @code{.cpp}
     * vk::Extent2D swapchainExtent = ...;
     * cb.setViewport(0, vku::toViewport(swapchainExtent, true)); // Use negative viewport height.
     * cb.setScissor(0, vk::Rect2D { { 0, 0 }, swapchainExtent });
     * @endcode
     *
     * @param extent Extent to convert.
     * @param negativeHeight Whether the height is negative. Default is <tt>false</tt>.
     * @return Converted viewport.
     * @note For about negative viewport height, see https://www.saschawillems.de/blog/2019/03/29/flipping-the-vulkan-viewport/
     * for detail.
     */
    export
    [[nodiscard]] constexpr VULKAN_HPP_NAMESPACE::Viewport toViewport(const VULKAN_HPP_NAMESPACE::Extent2D &extent, bool negativeHeight = false) noexcept {
        if (negativeHeight) {
            return { 0.f, static_cast<float>(extent.height), static_cast<float>(extent.width), -static_cast<float>(extent.height), 0.f, 1.f };
        }
        else {
            return { 0.f, 0.f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.f, 1.f };
        }
    }

    /**
     * @brief Cast the Vulkan object handle into its corresponding C handle.
     *
     * @code{.cpp}
     * vku::AllocatedImage image { device, vk::ImageCreateInfo { ... } };
     * toCType<vk::Image>(image); // VkImage struct. Image is first implicitly converted to vk::Image, and passed as the parameter.
     * vk::raii::ImageView imageView { device, image.getViewCreateInfo { ... } };
     * toCType(*imageView); // VkImageView struct. Handle type is automatically inferred.
     * @endcode
     *
     * @tparam T Type of Vulkan handle.
     * @param handle Vulkan object handle.
     * @return The corresponding inner C handle of \p handle.
     */
    export template <typename T>
    [[nodiscard]] typename T::CType toCType(T handle) noexcept {
        return static_cast<typename T::CType>(handle);
    }

    /**
     * @brief Get the 64-bit GPU address of the Vulkan object.
     *
     * @code{.cpp}
     * vku::AllocatedImage image { device, vk::ImageCreateInfo { ... } };
     * toUint64<vk::Image>(image); // Image is first implicitly converted to vk::Image, and passed as the parameter.
     * vk::raii::ImageView imageView { device, image.getViewCreateInfo { ... } };
     * toUint64(*imageView); // Handle type is automatically inferred.
     * @endcode
     *
     * @tparam T Type of Vulkan handle.
     * @param handle Vulkan object handle.
     * @return The 64-bit GPU address.
     */
    export template <typename T>
    [[nodiscard]] std::uint64_t toUint64(T handle) noexcept {
        return reinterpret_cast<std::uint64_t>(toCType(handle));
    }

    /**
     * @brief Get aspect ratio (width / height) of the 2-dimensional extent.
     * @tparam T Floating point type to calculate the aspect ratio. Default is <tt>float</tt>.
     * @param extent Extent to calculate the aspect ratio. Its height must not be zero.
     * @return Aspect ratio of the extent.
     */
    export template <std::floating_point T = float>
    [[nodiscard]] constexpr T aspect(const VULKAN_HPP_NAMESPACE::Extent2D &extent) NOEXCEPT_IF_RELEASE {
        assert(extent.height != 0 && "Height must be nonzero.");
        return static_cast<T>(extent.width) / static_cast<T>(extent.height);
    }
}