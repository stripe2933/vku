#pragma once

#include <type_traits>

#include "macros.hpp"
#define VKU_FWD_LIKE(T, ...) static_cast<typename FwdLike<::std::is_lvalue_reference_v<T>>::template type<decltype(__VA_ARGS__)>>(__VA_ARGS__)

namespace vku::details {
    template <bool> struct FwdLike { template <class T> using type = std::remove_reference_t<T>&&; };
    template <> struct FwdLike<true> { template <class T> using type = std::remove_reference_t<T>&; }; // to speed up compilation times

    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&&,   std::integral_constant<std::size_t, 0>) noexcept { return VKU_FWD(fn)(); }
    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, std::integral_constant<std::size_t, 1>) noexcept { auto&& [_1] = VKU_FWD(t); return VKU_FWD(fn)(VKU_FWD_LIKE(T, _1)); }
    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, std::integral_constant<std::size_t, 2>) noexcept { auto&& [_1, _2] = VKU_FWD(t); return VKU_FWD(fn)(VKU_FWD_LIKE(T, _1), VKU_FWD_LIKE(T, _2)); }
    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, std::integral_constant<std::size_t, 3>) noexcept { auto&& [_1, _2, _3] = VKU_FWD(t); return VKU_FWD(fn)(VKU_FWD_LIKE(T, _1), VKU_FWD_LIKE(T, _2), VKU_FWD_LIKE(T, _3)); }
    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, std::integral_constant<std::size_t, 4>) noexcept { auto&& [_1, _2, _3, _4] = VKU_FWD(t); return VKU_FWD(fn)(VKU_FWD_LIKE(T, _1), VKU_FWD_LIKE(T, _2), VKU_FWD_LIKE(T, _3), VKU_FWD_LIKE(T, _4)); }
    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, std::integral_constant<std::size_t, 5>) noexcept { auto&& [_1, _2, _3, _4, _5] = VKU_FWD(t); return VKU_FWD(fn)(VKU_FWD_LIKE(T, _1), VKU_FWD_LIKE(T, _2), VKU_FWD_LIKE(T, _3), VKU_FWD_LIKE(T, _4), VKU_FWD_LIKE(T, _5)); }
    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, std::integral_constant<std::size_t, 6>) noexcept { auto&& [_1, _2, _3, _4, _5, _6] = VKU_FWD(t); return VKU_FWD(fn)(VKU_FWD_LIKE(T, _1), VKU_FWD_LIKE(T, _2), VKU_FWD_LIKE(T, _3), VKU_FWD_LIKE(T, _4), VKU_FWD_LIKE(T, _5), VKU_FWD_LIKE(T, _6)); }
    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, std::integral_constant<std::size_t, 7>) noexcept { auto&& [_1, _2, _3, _4, _5, _6, _7] = VKU_FWD(t); return VKU_FWD(fn)(VKU_FWD_LIKE(T, _1), VKU_FWD_LIKE(T, _2), VKU_FWD_LIKE(T, _3), VKU_FWD_LIKE(T, _4), VKU_FWD_LIKE(T, _5), VKU_FWD_LIKE(T, _6), VKU_FWD_LIKE(T, _7)); }
    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, std::integral_constant<std::size_t, 8>) noexcept { auto&& [_1, _2, _3, _4, _5, _6, _7, _8] = VKU_FWD(t); return VKU_FWD(fn)(VKU_FWD_LIKE(T, _1), VKU_FWD_LIKE(T, _2), VKU_FWD_LIKE(T, _3), VKU_FWD_LIKE(T, _4), VKU_FWD_LIKE(T, _5), VKU_FWD_LIKE(T, _6), VKU_FWD_LIKE(T, _7), VKU_FWD_LIKE(T, _8)); }
    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, std::integral_constant<std::size_t, 9>) noexcept { auto&& [_1, _2, _3, _4, _5, _6, _7, _8, _9] = VKU_FWD(t); return VKU_FWD(fn)(VKU_FWD_LIKE(T, _1), VKU_FWD_LIKE(T, _2), VKU_FWD_LIKE(T, _3), VKU_FWD_LIKE(T, _4), VKU_FWD_LIKE(T, _5), VKU_FWD_LIKE(T, _6), VKU_FWD_LIKE(T, _7), VKU_FWD_LIKE(T, _8), VKU_FWD_LIKE(T, _9)); }
    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, std::integral_constant<std::size_t, 10>) noexcept { auto&& [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10] = VKU_FWD(t); return VKU_FWD(fn)(VKU_FWD_LIKE(T, _1), VKU_FWD_LIKE(T, _2), VKU_FWD_LIKE(T, _3), VKU_FWD_LIKE(T, _4), VKU_FWD_LIKE(T, _5), VKU_FWD_LIKE(T, _6), VKU_FWD_LIKE(T, _7), VKU_FWD_LIKE(T, _8), VKU_FWD_LIKE(T, _9), VKU_FWD_LIKE(T, _10)); }
    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, std::integral_constant<std::size_t, 11>) noexcept { auto&& [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11] = VKU_FWD(t); return VKU_FWD(fn)(VKU_FWD_LIKE(T, _1), VKU_FWD_LIKE(T, _2), VKU_FWD_LIKE(T, _3), VKU_FWD_LIKE(T, _4), VKU_FWD_LIKE(T, _5), VKU_FWD_LIKE(T, _6), VKU_FWD_LIKE(T, _7), VKU_FWD_LIKE(T, _8), VKU_FWD_LIKE(T, _9), VKU_FWD_LIKE(T, _10), VKU_FWD_LIKE(T, _11)); }
    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, std::integral_constant<std::size_t, 12>) noexcept { auto&& [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12] = VKU_FWD(t); return VKU_FWD(fn)(VKU_FWD_LIKE(T, _1), VKU_FWD_LIKE(T, _2), VKU_FWD_LIKE(T, _3), VKU_FWD_LIKE(T, _4), VKU_FWD_LIKE(T, _5), VKU_FWD_LIKE(T, _6), VKU_FWD_LIKE(T, _7), VKU_FWD_LIKE(T, _8), VKU_FWD_LIKE(T, _9), VKU_FWD_LIKE(T, _10), VKU_FWD_LIKE(T, _11), VKU_FWD_LIKE(T, _12)); }
    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, std::integral_constant<std::size_t, 13>) noexcept { auto&& [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13] = VKU_FWD(t); return VKU_FWD(fn)(VKU_FWD_LIKE(T, _1), VKU_FWD_LIKE(T, _2), VKU_FWD_LIKE(T, _3), VKU_FWD_LIKE(T, _4), VKU_FWD_LIKE(T, _5), VKU_FWD_LIKE(T, _6), VKU_FWD_LIKE(T, _7), VKU_FWD_LIKE(T, _8), VKU_FWD_LIKE(T, _9), VKU_FWD_LIKE(T, _10), VKU_FWD_LIKE(T, _11), VKU_FWD_LIKE(T, _12), VKU_FWD_LIKE(T, _13)); }
    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, std::integral_constant<std::size_t, 14>) noexcept { auto&& [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14] = VKU_FWD(t); return VKU_FWD(fn)(VKU_FWD_LIKE(T, _1), VKU_FWD_LIKE(T, _2), VKU_FWD_LIKE(T, _3), VKU_FWD_LIKE(T, _4), VKU_FWD_LIKE(T, _5), VKU_FWD_LIKE(T, _6), VKU_FWD_LIKE(T, _7), VKU_FWD_LIKE(T, _8), VKU_FWD_LIKE(T, _9), VKU_FWD_LIKE(T, _10), VKU_FWD_LIKE(T, _11), VKU_FWD_LIKE(T, _12), VKU_FWD_LIKE(T, _13), VKU_FWD_LIKE(T, _14)); }
    template <class Fn, class T> [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, std::integral_constant<std::size_t, 15>) noexcept { auto&& [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15] = VKU_FWD(t); return VKU_FWD(fn)(VKU_FWD_LIKE(T, _1), VKU_FWD_LIKE(T, _2), VKU_FWD_LIKE(T, _3), VKU_FWD_LIKE(T, _4), VKU_FWD_LIKE(T, _5), VKU_FWD_LIKE(T, _6), VKU_FWD_LIKE(T, _7), VKU_FWD_LIKE(T, _8), VKU_FWD_LIKE(T, _9), VKU_FWD_LIKE(T, _10), VKU_FWD_LIKE(T, _11), VKU_FWD_LIKE(T, _12), VKU_FWD_LIKE(T, _13), VKU_FWD_LIKE(T, _14), VKU_FWD_LIKE(T, _15)); }

    template <class T> extern const T ext{};
    struct any { template <class T> operator T() const noexcept; };
    template<auto...> struct auto_ { constexpr auto_(auto&&...) noexcept { } };

    template <std::size_t N, class...Ts> requires (N < sizeof...(Ts))
    [[nodiscard]] constexpr decltype(auto) nth_pack_element(Ts&&...args) noexcept {
        return VKU_INDEX_SEQ(Ns, N, {
            return [](auto_<Ns>&&..., auto&& nth, auto&&...) -> decltype(auto) { return VKU_FWD(nth); };
        })(VKU_FWD(args)...);
    }

    template <class T, class... Ts>
        requires std::is_aggregate_v<T>
    [[nodiscard]] constexpr auto size() -> std::size_t {
        if constexpr (requires { T{Ts{}...}; } and not requires { T{Ts{}..., any{}}; }) {
            return sizeof...(Ts);
        } else {
            return size<T, Ts..., any>();
        }
    }

    template <class Fn, class T>
        requires std::is_aggregate_v<std::remove_cvref_t<T>>
    [[nodiscard]] constexpr decltype(auto) visit(Fn&& fn, T&& t, auto...) noexcept {
#if __cpp_structured_bindings >= 202401L
        auto&& [... ts] = VKU_FWD(t);
          return VKU_FWD(fn)(VKU_FWD_LIKE(T, ts)...);
#else
          return visit(VKU_FWD(fn), VKU_FWD(t), std::integral_constant<std::size_t, size<std::remove_cvref_t<T>>()>{});
#endif
    }

    template<std::size_t N, class T> requires (std::is_aggregate_v<std::remove_cvref_t<T>> and N < size<std::remove_cvref_t<T>>())
    [[nodiscard]] constexpr decltype(auto) get(T&& t) noexcept {
        return visit([](auto&&... args) -> decltype(auto) { return nth_pack_element<N>(VKU_FWD(args)...); }, VKU_FWD(t));
    }

    template <std::size_t N, class T>
        requires std::is_aggregate_v<std::remove_cvref_t<T>>
    [[nodiscard]] constexpr auto size_of() -> std::size_t {
        return sizeof(std::remove_cvref_t<decltype(get<N>(ext<T>))>);
    }

    template <std::size_t N, class T>
        requires std::is_aggregate_v<std::remove_cvref_t<T>>
    [[nodiscard]] constexpr auto size_of(T&&) -> std::size_t {
        return size_of<N, std::remove_cvref_t<T>>();
    }

    template <std::size_t N, class T>
        requires std::is_aggregate_v<std::remove_cvref_t<T>>
    [[nodiscard]] constexpr auto align_of() -> std::size_t {
        return alignof(std::remove_cvref_t<decltype(get<N>(ext<T>))>);
    }

    template <std::size_t N, class T>
        requires std::is_aggregate_v<std::remove_cvref_t<T>>
    [[nodiscard]] constexpr auto align_of(T&&) -> std::size_t {
        return align_of<N, std::remove_cvref_t<T>>();
    }

    template <std::size_t N, class T>
        requires std::is_aggregate_v<std::remove_cvref_t<T>>
    [[nodiscard]] constexpr auto offset_of() -> std::size_t {
        if constexpr (not N) {
            return {};
        } else {
            constexpr auto offset = offset_of<N-1, T>() + size_of<N-1, T>();
            constexpr auto alignment = std::min(alignof(T), align_of<N, T>());
            constexpr auto padding = offset % alignment;
            return offset + padding;
        }
    }

    template <std::size_t N, class T>
        requires std::is_aggregate_v<std::remove_cvref_t<T>>
    [[nodiscard]] constexpr auto offset_of(T&&) -> std::size_t {
        return offset_of<N, std::remove_cvref_t<T>>();
    }

    template <class T, class Fn>
        requires std::is_aggregate_v<std::remove_cvref_t<T>>
    constexpr auto for_each(Fn&& fn) -> void {
        VKU_INDEX_SEQ(Ns, size<std::remove_cvref_t<T>>(), {
            (VKU_FWD(fn)(std::integral_constant<decltype(Ns), Ns>{}), ...);
        });
    }

    template <class Fn, class T>
        requires std::is_aggregate_v<std::remove_cvref_t<T>>
    constexpr auto for_each(Fn&& fn, T&&) -> void {
        VKU_INDEX_SEQ(Ns, size<std::remove_cvref_t<T>>(), {
            (VKU_FWD(fn)(std::integral_constant<decltype(Ns), Ns>{}), ...);
        });
    }
}