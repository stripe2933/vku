#pragma once

#include <ranges>
#include <version>

#include "macros.hpp"

// TODO: remove when MSVC supports static call operator.
#if __cpp_static_call_operator >= 202207L
#define VKU_STATIC_CALL_OPERATOR_STATIC static
#define VKU_STATIC_CALL_OPERATOR_CONST
#else
#define VKU_STATIC_CALL_OPERATOR_STATIC
#define VKU_STATIC_CALL_OPERATOR_CONST const
#endif

namespace details::ranges {
    // --------------------
    // Ranges.
    // --------------------

    template <typename Derived>
    struct range_adaptor_closure {
        template <std::ranges::range R>
        friend constexpr auto operator|(
            R &&r,
            const Derived &derived
        ) noexcept(std::is_nothrow_invocable_v<const Derived&, R>) {
            return derived(VKU_FWD(r));
        }
    };

    template <std::size_t N>
    struct to_array : range_adaptor_closure<to_array<N>> {
        template <std::ranges::input_range R>
        VKU_STATIC_CALL_OPERATOR_STATIC constexpr auto operator()(
            R &&r
        ) VKU_STATIC_CALL_OPERATOR_CONST -> std::array<std::ranges::range_value_t<R>, N> {
            auto it = r.begin();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
            return VKU_ARRAY_OF(N, *it++);
#pragma clang diagnostic pop
        }
    };

    namespace views {
#if __cpp_lib_ranges_zip >= 202110L
        constexpr decltype(std::views::zip_transform) zip_transform;
#else
        constexpr auto zip_transform(
            auto &&f,
            std::ranges::input_range auto &&...rs
        ) -> auto {
            return std::views::zip(VKU_FWD(rs)...) | std::views::transform([&](auto &&t) {
                return std::apply(f, VKU_FWD(t));
            });
        }
#endif
    }
}