/** @file details/functional.cppm
 */

export module vku.details:functional;

namespace vku::details {
    export template <typename ...Fs>
    struct multilambda : Fs... {
        using Fs::operator()...;
    };
}