module;

#ifndef VKU_USE_STD_MODULE
#include <format>
#include <source_location>
#include <string>
#endif

export module vku:details.to_string;

#ifdef VKU_USE_STD_MODULE
import std;
#endif

namespace details {
    export
    [[nodiscard]] auto to_string(std::source_location srcLoc) noexcept -> std::string {
        return std::format("{}:{}:{}", srcLoc.file_name(), srcLoc.line(), srcLoc.column());
    }
}