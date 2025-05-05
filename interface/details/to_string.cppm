/** @file details/to_string.cppm
 */

export module vku.details:to_string;

import std;

namespace vku::details {
    export
    [[nodiscard]] std::string to_string(std::source_location srcLoc) noexcept {
        return std::format("{}:{}:{}", srcLoc.file_name(), srcLoc.line(), srcLoc.column());
    }
}