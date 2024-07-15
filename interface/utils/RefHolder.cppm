export module vku:utils.RefHolder;

import std;

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

namespace vku{
    /**
     * <tt>RefHolder<T, Ts...></tt> is a container that can be contextually converted to a reference of type <tt>T</tt>,
     * which has references for the types <tt>Ts...</tt>.
     * @tparam T A type that represents the value of <tt>RefHolder</tt>.
     * @tparam Ts Types that are used as references for the value of <tt>RefHolder</tt>.
     * @note
     * This class is intended to be used in a situation for returning the value that has references to the various types,
     * but they doesn't have to be emphasized (function users don't have to know about them, they only care about the value).
     *
     * TODO: add more description and examples.
     */
    export template <typename T, typename... Ts>
    struct RefHolder {
        std::tuple<Ts...> temporaryValues;
        T value;

        template <std::invocable<Ts&...> F>
        RefHolder(
            F &&f,
            auto &&...temporaryValues
        ) : temporaryValues { FWD(temporaryValues)... },
            value { std::apply(FWD(f), this->temporaryValues) } { }

        [[nodiscard]] constexpr operator T&() noexcept { return value; }
        [[nodiscard]] constexpr operator const T&() const noexcept { return value; }

        [[nodiscard]] constexpr auto get() noexcept -> T& { return value; }
        [[nodiscard]] constexpr auto get() const noexcept -> const T& { return value; }
    };

    // --------------------
    // Type deduction guides.
    // --------------------

    template <typename F, typename... Ts>
    RefHolder(F&&, Ts&&...) -> RefHolder<std::invoke_result_t<F, Ts&...>, Ts...>;
}