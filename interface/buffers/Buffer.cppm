/** @file buffers/Buffer.cppm
 */

module;

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:buffers.Buffer;

import std;
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

        // --------------------
        // Member functions.
        // --------------------

        /**
         * @brief <tt>vk::BufferViewCreateInfo</tt> struct with the specified \p format and range(\p offset, \p range).
         * @param format Format of the buffer view.
         * @param offset Offset in bytes from the start of the buffer. Default is 0.
         * @param range Range in bytes of the buffer view. Default is the whole size of the buffer.
         * @return <tt>vk::BufferViewCreateInfo</tt> struct.
         */
        [[nodiscard]] VULKAN_HPP_NAMESPACE::BufferViewCreateInfo getViewCreateInfo(
            VULKAN_HPP_NAMESPACE::Format format,
            VULKAN_HPP_NAMESPACE::DeviceSize offset = 0,
            VULKAN_HPP_NAMESPACE::DeviceSize range = VULKAN_HPP_NAMESPACE::WholeSize
        ) const noexcept {
            return { {}, buffer, format, offset, range };
        }
    };
}