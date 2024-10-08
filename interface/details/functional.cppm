/** @file details/functional.cppm
 */
export module vku:details.functional;

namespace details {
    export template <typename ...Fs>
    struct multilambda : Fs... {
        using Fs::operator()...;
    };
}