module;

#ifndef VKU_USE_STD_MODULE
#include <initializer_list>
#include <ranges>
#endif

export module vku:utils;
export import :utils.RefHolder;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vulkan_hpp;

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
     * Creating <tt>vk::ImageSubresourceRange</tt> that contains the whole mip levels and array layers, with specified
     * \p aspectFlags.
     * @param aspectFlags Image aspect. Default is <tt>vk::ImageAspectFlagBits::eColor</tt>.
     * @return <tt>vk::ImageSubresourceRange</tt> that contains the whole mip levels and array layers.
     */
    export
    [[nodiscard]] constexpr auto fullSubresourceRange(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor) noexcept -> vk::ImageSubresourceRange {
        return { aspectFlags, 0, vk::RemainingMipLevels, 0, vk::RemainingArrayLayers };
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
}