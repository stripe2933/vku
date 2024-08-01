export module vku:details.functional;

namespace vku::inline functional {
    export template <typename ...Fs>
    struct multilambda : Fs... {
        using Fs::operator()...;
    };
}