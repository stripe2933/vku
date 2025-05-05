/** @file debugging.cppm
 */

module;

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:debugging;

import std;
import vku.details;
import :utils;

namespace vku {
    /**
     * @brief <tt>vk::DebugUtilsObjectNameInfoEXT</tt> with deduced object type.
     * @tparam T Vulkan object type.
     * @param handle Vulkan object handle.
     * @param name Name of the object to be set. Default is the current source location.
     * @return <tt>vk::DebugUtilsObjectNameInfoEXT</tt> with deduced object type.
     */
    export template <typename T>
    [[nodiscard]] VULKAN_HPP_NAMESPACE::DebugUtilsObjectNameInfoEXT getDebugUtilsObjectNameInfoEXT(
        T handle,
        const char *name = details::to_string(std::source_location::current()).c_str()
    ) noexcept {
        return { T::objectType, toUint64(handle), name };
    }

    /**
     * @brief <tt>vk::DebugUtilsObjectTagInfoEXT</tt> with deduced object type and tag data.
     * @tparam T Vulkan object type.
     * @tparam U Data type of the tag.
     * @param handle Vulkan object handle.
     * @param data Contiguous range of the data to be tagged.
     * @param name Name of the object to be set. Default is the current source location.
     * @return <tt>vk::DebugUtilsObjectTagInfoEXT</tt> with deduced object type and tag data.
     */
    export template <typename T, typename U>
    [[nodiscard]] VULKAN_HPP_NAMESPACE::DebugUtilsObjectTagInfoEXT getDebugUtilsObjectTagInfoEXT(
        T handle,
        VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const U> data,
        const char *name = details::to_string(std::source_location::current()).c_str()
    ) noexcept {
        return { T::objectType, toUint64(handle), name, data };
    }
}