module;

#include <cassert>
#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <compare>
#include <concepts>
#include <initializer_list>
#include <ranges>
#include <utility>
#endif

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

namespace vku {
    /**
     * Take a temporary value and return its address. The result address is only valid until the end of the expression,
     * which contains the temporary value.
     * @param value Temporary value.
     * @return Address of the temporary value.
     * @note In Vulkan-Hpp API, you can use this function to pass temporary values to functions that require a pointer.
     * For example, the following code creating a Vulkan instance with an application info
     * @code
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
     * @code
     * vk::raii::Instance instance { context, vk::InstanceCreateInfo {
     *     {},
     *     vku::unsafeAddress(vk::ApplicationInfo { ... }),
     * } }; // vk::ApplicationInfo destroyed here, however vk::raii::Instance already created and it's fine.
     * @endcode
     */
    export template <typename T>
    [[nodiscard]] auto unsafeAddress(const T &value [[clang::lifetimebound]]) noexcept -> const T* {
        return &value;
    }

    /**
     * Take a temporary value and return <tt>vk::ArrayProxyNoTemporaries</tt> of it. The result is only valid until the
     * end of the expression, which contains the temporary value.
     * @param value Temporary value.
     * @return <tt>vk::ArrayProxyNoTemporaries</tt> of the temporary value.
     * @note The intention of this function follows the same reason as <tt>unsafeAddress</tt> function.
     * @example
     * @code
     * vk::raii::Device device { physicalDevice, vk::DeviceCreateInfo {
     *     {},
     *     // { vk::DeviceQueueCreateInfo { ... } }, // ERROR! calling deleted constructor of vk::ArrayProxyNoTemporaries<const vk::DeviceQueueCreateInfo>.
     *     vku::unsafeProxy(vk::DeviceQueueCreateInfo { ... }), // OK. Just make ensure that vk::DeviceQueueCreateInfo alives until the end of the device creation.
     *     ...
     * };
     * @endcode
     */
    export template <typename T>
    [[nodiscard]] auto unsafeProxy(const T &value [[clang::lifetimebound]]) noexcept -> vk::ArrayProxyNoTemporaries<const T> {
        return value;
    }

    /**
     * Take a temporary contiguous range and return <tt>vk::ArrayProxyNoTemporaries</tt> of it. The result is only valid
     * until the end of the expression, which contains the temporary range.
     * @param range Temporary contiguous range.
     * @return <tt>vk::ArrayProxyNoTemporaries</tt> of the temporary range.
     * @note The intention of this function follows the same reason as <tt>unsafeAddress</tt> function.
     */
    export template <std::ranges::contiguous_range R>
    [[nodiscard]] auto unsafeProxy(const R &range [[clang::lifetimebound]]) noexcept -> vk::ArrayProxyNoTemporaries<const std::ranges::range_value_t<R>> {
        return range;
    }

    /**
     * Take a temporary initializer list and return <tt>vk::ArrayProxyNoTemporaries</tt> of it. The result is only valid
     * until the end of the expression, which contains the temporary initializer list.
     * @param list Temporary initializer list.
     * @return <tt>vk::ArrayProxyNoTemporaries</tt> of the temporary initializer list.
     * @note The intention of this function follows the same reason as <tt>unsafeAddress</tt> function.
     * @example
     * @code
     * vk::raii::DescritorSetLayout device { device, vk::DescriptorSetLayoutCreateInfo {
     *     {},
     *     // { vk::DescriptorSetLayoutBinding { ... }, ... }, // ERROR! calling deleted constructor of vk::ArrayProxyNoTemporaries<const vk::DescriptorSetLayoutBinding>.
     *     vku::unsafeProxy({
     *         vk::DescriptorSetLayoutBinding { ... },
     *         vk::DescriptorSetLayoutBinding { ... },
     *         ...
     *     }), // OK. Just make ensure that vk::DescriptorSetLayoutBindings alive until the end of the descriptor set layout creation.
     * };
     * @endcode
     */
    export template <typename T>
    [[nodiscard]] auto unsafeProxy(const std::initializer_list<T> &list [[clang::lifetimebound]]) noexcept -> vk::ArrayProxyNoTemporaries<const T> {
        return list;
    }

    /**
     * Check whether \p flag is contained in \p flags.
     * @param flags Vulkan flags.
     * @param flag Vulkan flag bit.
     * @return <tt>true</tt> If flag is contained in flags, <tt>false</tt> otherwise.
     * @see contains(vk::Flags<T>, vk::Flags<T>) -> bool
     */
    export template <typename T>
    [[nodiscard]] constexpr auto contains(vk::Flags<T> flags, T flag) noexcept -> bool {
        return (flags & flag) == flag;
    }

    /**
     * Check whether \p sub is contained in \p super.
     * @param super Vulkan flags.
     * @param sub Vulkan flags.
     * @return <tt>true</tt> If sub is contained in super, <tt>false</tt> otherwise.
     * @see contains(vk::Flags<T>, T) -> bool
     */
    export template <typename T>
    [[nodiscard]] constexpr auto contains(vk::Flags<T> super, vk::Flags<T> sub) noexcept -> bool {
        return (super & sub) == sub;
    }

    /**
     * Convert <tt>vk::Extent3D</tt> to <tt>vk::Extent2D</tt>. The depth component is discarded.
     * @param extent Extent to convert.
     * @return Converted extent.
     */
    export
    [[nodiscard]] constexpr auto toExtent2D(const vk::Extent3D &extent) noexcept -> vk::Extent2D {
        return { extent.width, extent.height };
    }

    /**
     * Convert <tt>vk::Offset2D</tt> to <tt>vk::Extent2D</tt>. Negative component is converted to the least unsigned
     * integer congruent to the source integer.
     * @param offset Offset to convert.
     * @return Converted extent.
     * @note Negative component will be casted to unsigned int, with C++ standard conversion rule.
     */
    export
    [[nodiscard]] constexpr auto toExtent2D(const vk::Offset2D &offset) noexcept -> vk::Extent2D {
        return { static_cast<std::uint32_t>(offset.x), static_cast<std::uint32_t>(offset.y) };
    }

    /**
     * Convert <tt>vk::Offset3D</tt> to <tt>vk::Offset2D</tt>. The z component is discarded.
     * @param offset Offset to convert.
     * @return Converted offset.
     */
    export
    [[nodiscard]] constexpr auto toOffset2D(const vk::Offset3D &offset) noexcept -> vk::Offset2D {
        return { offset.x, offset.y };
    }

    /**
     * Convert <tt>vk::Extent2D</tt> to <tt>vk::Offset2D</tt>.
     * @param extent Extent to convert.
     * @return Converted offset.
     * @throw
     * - Assertion error if width or height is overflowing (> 2^31 - 1).
     * @note Signed integer overflow is UB in C++. Make sure that the width and height are less than 2^31 - 1.
     */
    export
    [[nodiscard]] constexpr auto toOffset2D(const vk::Extent2D &extent) NOEXCEPT_IF_RELEASE -> vk::Offset2D {
        assert(std::in_range<std::int32_t>(extent.width) && "Overflowing width.");
        assert(std::in_range<std::int32_t>(extent.height) && "Overflowing height.");
        return { static_cast<std::int32_t>(extent.width), static_cast<std::int32_t>(extent.height) };
    }

    /**
     * Convert <tt>vk::Extent2D</tt> to <tt>vk::Viewport</tt>, with depth=0..1 and additional negative height flag.
     * @param extent Extent to convert.
     * @param negativeHeight Whether the height is negative. Default is <tt>false</tt>.
     * @return Converted viewport.
     * @note
     * - For about negative viewport height, see https://www.saschawillems.de/blog/2019/03/29/flipping-the-vulkan-viewport/ for detail.
     * @example
     * Setting viewport and scissor as full region of the swapchain for pipeline dynamic state:
     * @code
     * vk::Extent2D swapchainExtent = ...;
     * cb.setViewport(0, vku::toViewport(swapchainExtent, true)); // Use negative viewport height.
     * cb.setScissor(0, vk::Rect2D { { 0, 0 }, swapchainExtent });
     * @endcode
     */
    export
    [[nodiscard]] constexpr auto toViewport(const vk::Extent2D &extent, bool negativeHeight = false) noexcept -> vk::Viewport {
        return negativeHeight
            ? vk::Viewport { 0.f, static_cast<float>(extent.height), static_cast<float>(extent.width), -static_cast<float>(extent.height), 0.f, 1.f }
            : vk::Viewport { 0.f, 0.f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.f, 1.f };
    }

    /**
     * Get aspect ratio (width / height) of the extent.
     * @tparam T Floating point type to calculate the aspect ratio. Default is <tt>float</tt>.
     * @param extent Extent to calculate the aspect ratio.
     * @return Aspect ratio of the extent.
     * @throw
     * - Assertion error if height is zero (zero division).
     */
    export template <std::floating_point T = float>
    [[nodiscard]] constexpr auto aspect(const vk::Extent2D &extent) NOEXCEPT_IF_RELEASE -> T {
        assert(extent.height != 0 && "Height must not be zero.");
        return static_cast<T>(extent.width) / static_cast<T>(extent.height);
    }
}