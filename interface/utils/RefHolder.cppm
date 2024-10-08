/** @file utils/RefHolder.cppm
 */

module;

#ifndef VKU_USE_STD_MODULE
#include <concepts>
#include <functional>
#include <tuple>
#include <type_traits>
#endif

export module vku:utils.RefHolder;

#ifdef VKU_USE_STD_MODULE
import std;
#endif

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

namespace vku{
    /**
     * @brief A container type that can be contextually converted to a reference of type <tt>T</tt>, which has references for instances of the types <tt>Ts...</tt>.
     *
     * This class is intended to be used in a situation for returning the value that has references to the various types, but they don't have to be emphasized (function users don't have to know about them, they only care about the value).
     *
     * @tparam T A type that represents the value of <tt>RefHolder</tt>.
     * @tparam Ts Types that are used as references for the value of <tt>RefHolder</tt>.
     */
    export template <typename T, typename... Ts>
    class RefHolder {
        std::tuple<Ts...> temporaryValues;

    public:
        T value;

        template <std::invocable<Ts&...> F>
        explicit(sizeof...(Ts) == 0) RefHolder(
            F &&f,
            auto &&...temporaryValues
        ) : temporaryValues { FWD(temporaryValues)... },
            value { std::apply(FWD(f), this->temporaryValues) } { }

        /**
         * Make this struct implicitly convertible to <tt>T&</tt>.
         */
        [[nodiscard]] constexpr operator T&() noexcept { return value; }

        /**
         * Make this struct implicitly convertible to <tt>const T&</tt>.
         */
        [[nodiscard]] constexpr operator const T&() const noexcept { return value; }

        /**
         * @brief Get the reference of the value.
         * @return Reference of the value.
         */
        [[nodiscard]] constexpr T& get() noexcept { return value; }

        /**
         * @brief Get the const reference of the value.
         * @return Const reference of the value.
         */
        [[nodiscard]] constexpr const T& get() const noexcept { return value; }
    };

    // --------------------
    // Type deduction guides.
    // --------------------

    template <typename F, typename... Ts>
    RefHolder(F&&, Ts&&...) -> RefHolder<std::invoke_result_t<F, Ts&...>, Ts...>;
}