module;

#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <tuple>
#include <utility>
#endif

export module vku:details.tuple;

#ifdef VKU_USE_STD_MODULE
import std;
#endif

#define INDEX_SEQ(Is, N, ...) [&]<std::size_t ...Is>(std::index_sequence<Is...>) __VA_ARGS__ (std::make_index_sequence<N>{})
#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

namespace details {
    /**
     * Invoke the function with tuple elements.
     * @code{.cpp}
     * apply_by_value([](auto &&value) {
     *     std::println("{}", FWD(value));
     * }, std::forward_as_tuple(1, 2.f, "Hello world!"));
     *
     * // [Expected output]
     * // 1
     * // 2
     * // Hello world!
     * @endcode
     * @param f Function to apply, must be invocable with all tuple element types.
     * @param tuple Tuple to apply.
     */
    export auto apply_by_value(const auto &f, auto &&tuple) -> void {
        std::apply([&](auto &&...xs) {
            (f(FWD(xs)), ...);
        }, FWD(tuple));
    }

    /**
     * Invoke the function with tuple elements and their indices.
     * @code{.cpp}
     * apply_with_index([]<std::size_t N>(std::integral_constant<std::size_t, N>, auto &&value) {
     *     std::println("{} {}", N, FWD(value));
     * }, std::forward_as_tuple(1, 2.f, "Hello world!"));
     *
     * // [Expected output]
     * // 0 1
     * // 1 2
     * // 2 Hello world!
     * @endcode
     * @param f Function to apply, must be invocable with std::integral_constant<std::size_t, N> and N-th tuple element type.
     * @param tuple Tuple to apply.
     */
    export auto apply_with_index(const auto &f, auto &&tuple) -> void {
        std::apply([&](auto &&...xs) {
            INDEX_SEQ(Is, sizeof...(xs), {
                (f(std::integral_constant<std::size_t, Is>{}, FWD(xs)), ...);
            });
        }, FWD(tuple));
    }
}