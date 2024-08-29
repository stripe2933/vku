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
     * A thin wrapper around <tt>vk::Buffer</tt> with additional information such as buffer size in bytes. You can
     * construct this struct from an existing <tt>vk::Buffer</tt>.
     */
    export struct Buffer {
        VULKAN_HPP_NAMESPACE::Buffer buffer;
        VULKAN_HPP_NAMESPACE::DeviceSize size;

        // --------------------
        // User-defined conversion functions.
        // --------------------

        [[nodiscard]] operator VULKAN_HPP_NAMESPACE::Buffer() const noexcept; // can be implicitly converted to vk::Buffer.
    };
}

// --------------------
// Implementations.
// --------------------

vku::Buffer::operator VULKAN_HPP_NAMESPACE::Buffer() const noexcept {
    return buffer;
}