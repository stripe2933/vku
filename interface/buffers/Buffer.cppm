module;

#ifndef VKU_USE_STD_MODULE
#include <compare>
#endif

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
        vk::Buffer buffer;
        vk::DeviceSize size;

        // --------------------
        // User-defined conversion functions.
        // --------------------

        [[nodiscard]] operator vk::Buffer() const noexcept; // can be implicitly converted to vk::Buffer.
    };
}

// --------------------
// Implementations.
// --------------------

vku::Buffer::operator vk::Buffer() const noexcept {
    return buffer;
}