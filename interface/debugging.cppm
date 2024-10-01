module;

#ifndef VKU_USE_STD_MODULE
#include <source_location>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:debugging;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
import :details.to_string;
import :utils;

namespace vku {
    export template <typename T>
    [[nodiscard]] auto getDebugUtilsObjectNameInfoEXT(
        T handle,
        const char *name = to_string(std::source_location::current()).c_str()
    ) noexcept -> VULKAN_HPP_NAMESPACE::DebugUtilsObjectNameInfoEXT {
        return { T::objectType, toUint64(handle), name };
    }

    export template <typename T, typename U>
    [[nodiscard]] auto getDebugUtilsObjectTagInfoEXT(
        T handle,
        VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const U> data,
        const char *name = to_string(std::source_location::current()).c_str()
    ) noexcept -> VULKAN_HPP_NAMESPACE::DebugUtilsObjectTagInfoEXT {
        return { T::objectType, toUint64(handle), name, data };
    }
}