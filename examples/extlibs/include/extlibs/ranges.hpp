#pragma once

#include <ranges>
#include <version>

#define RANGES_FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

namespace ranges {
    template <typename Derived>
    struct range_adaptor_closure {
        template <std::ranges::range R>
        friend constexpr auto operator|(
            R &&r,
            const Derived &derived
        ) noexcept(std::is_nothrow_invocable_v<const Derived&, R>) {
            return derived(RANGES_FWD(r));
        }
    };

namespace views {
#if __cpp_lib_ranges_enumerate >= 202302L
    constexpr decltype(std::views::enumerate) enumerate;
#else
    struct enumerate_fn : range_adaptor_closure<enumerate_fn> {
        template <std::ranges::viewable_range R>
        static constexpr auto operator()(
            R &&r
        ) -> auto {
            return std::views::zip(
                std::views::iota(static_cast<std::ranges::range_difference_t<R>>(0)),
                RANGES_FWD(r));
        }
    };

    constexpr enumerate_fn enumerate;
#endif
}
}