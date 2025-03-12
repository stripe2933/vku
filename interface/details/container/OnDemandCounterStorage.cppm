/** @file details/container/OnDemandCounterStorage.cppm
 */

export module vku:details.container.OnDemandCounterStorage;

import std;

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

namespace details {
    /**
     * An associative container that creates the objects on demand, when the number of times the key type is accessed is
     * greater than the number of objects stored. The constructed objects are inserted into the <tt>std::deque</tt>'s end,
     * therefore the returning objects' references (by calling <tt>at(const Key&)</tt>) are preserved.
     * @code{.cpp}
     * OnDemandCounterStorage storage = makeOnDemandCounterStorage<int>([n = 0]() mutable { return n++; });
     * const int &a = storage.at(0); // construct() is called, a == 0.
     * const int &b = storage.at(0); // construct() is called, b == 1.
     * const int &c = storage.at(1); // Since there were already 2 constructed values and key 1 was never called before, c == a == 0.
     * const int &d = storage.at(1); // Since there were already 2 constructed values and key 1 was called once before, d == b == 1.
     * const int &e = storage.at(1); // construct() is called, e == 2.
     * assert(&a == &c); // a and c are the references to the first time constructed object by each key (0 and 1, respectively), therefore they are the same.
     * assert(&b == &d); // Same holds for b and d (second time constructed object).
     * @endcode
     * @tparam Key Key type to query.
     * @tparam ValueConstructor An invocable type for the value construction.
     * @tparam Value Value type to be constructed on demand, deduced by the return value of the <tt>ValueConstructor</tt>.
     * @see makeOnDemandCounterStorage for the type deduction helper function.
     */
    template <typename Key, typename ValueConstructor, typename Value = std::invoke_result_t<ValueConstructor>>
    class OnDemandCounterStorage {
    public:
        explicit OnDemandCounterStorage(ValueConstructor _valueConstructor) noexcept(std::is_nothrow_move_constructible_v<ValueConstructor>)
            : valueConstructor { std::move(_valueConstructor) } { }

        [[nodiscard]] auto at(const Key &k) noexcept(std::is_nothrow_invocable_v<ValueConstructor>) -> const Value& {
            if (auto index = counter[k]++; index < valueStorage.size()) {
                return valueStorage[index];
            }

            return valueStorage.emplace_back(valueConstructor());
        }

        [[nodiscard]] auto getValueStorage() const noexcept -> const std::deque<Value>& { return valueStorage; }
        [[nodiscard]] auto getValueStorage() noexcept -> std::deque<Value>& { return valueStorage; }

    private:
        ValueConstructor valueConstructor;
        std::deque<Value> valueStorage;
        std::unordered_map<Key, std::uint32_t> counter;
    };

    /**
     * Helper function for deducing the <tt>OnDemandCounterStorage</tt>'s template parameter types.
     * @code{.cpp}
     * // Without makeOnDemandCounterStorage:
     * const auto constructor = [n = 0]() mutable { return n++; };
     * OnDemandCounterStorage<int, std::remove_cvref_t<decltype(constructor)>> storage1 { constructor };
     *
     * // With makeOnDemandCounterStorage:
     * OnDemandCounterStorage storage2 = makeOnDemandCounterStorage<int>(constructor);
     * OnDemandCounterStorage storage3 = makeOnDemandCounterStorage<int>([n = 0]() mutable { return n++; });
     * @endcode
     * @tparam Key Key type to use. This is usually specified by the user.
     * @tparam ValueConstructor Value constructor type. Intended to be deduced by the function parameter \p valueConstructor.
     * @param valueConstructor A callable object that constructs values.
     * @return <tt>OnDemandCounterStorage</tt> with deduced type.
    */
    template <typename Key, typename ValueConstructor>
    [[nodiscard]] auto makeOnDemandCounterStorage(ValueConstructor valueConstructor) -> OnDemandCounterStorage<Key, ValueConstructor> {
        return OnDemandCounterStorage<Key, ValueConstructor> { std::move(valueConstructor) };
    }
}