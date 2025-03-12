/** @file details/to_string.cppm
 */

export module vku:details.to_string;

import std;

namespace details {
    export
    [[nodiscard]] auto to_string(std::source_location srcLoc) noexcept -> std::string {
        return std::format("{}:{}:{}", srcLoc.file_name(), srcLoc.line(), srcLoc.column());
    }
}