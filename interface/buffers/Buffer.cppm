/** @file buffers/Buffer.cppm
 */

module;

#if !defined(VKU_USE_STD_MODULE) && defined(_MSC_VER)
#include <compare>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:buffers.Buffer;

#if defined(VKU_USE_STD_MODULE) && defined(_MSC_VER)
import std;
#endif
export import vulkan_hpp;

namespace vku {
    /**
     * @brief Non-owning <tt>vk::Buffer</tt> handle with the additional information.
     */
    export struct Buffer {
        /**
         * @brief Vulkan handle that represents the buffer.
         */
        VULKAN_HPP_NAMESPACE::Buffer buffer;

        /**
         * @brief Buffer size in bytes.
         */
        VULKAN_HPP_NAMESPACE::DeviceSize size;

        // --------------------
        // User-defined conversion functions.
        // --------------------

        /**
         * Make this struct implicitly convertible to <tt>vk::Buffer</tt>.
         */
        [[nodiscard]] operator VULKAN_HPP_NAMESPACE::Buffer() const noexcept {
            return buffer;
        }
    };
}