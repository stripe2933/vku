/** @file commands.cppm
 */

module;

#include <version>

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:commands;

import std;
export import vulkan_hpp;
import :utils;

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)
#define INDEX_SEQ(Is, N, ...) [&]<std::size_t ...Is>(std::index_sequence<Is...>) __VA_ARGS__ (std::make_index_sequence<N>{})

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
void apply_by_value(const auto &f, auto &&tuple) {
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
void apply_with_index(const auto &f, auto &&tuple) {
    std::apply([&](auto &&...xs) {
        INDEX_SEQ(Is, sizeof...(xs), {
            (f(std::integral_constant<std::size_t, Is>{}, FWD(xs)), ...);
        });
    }, FWD(tuple));
}

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

namespace vku {
    /**
     * @brief Allocate compile-time amount of command buffers from \p commandPool.
     * @tparam N Number of command buffers to allocate.
     * @param device Vulkan device. Must be same device from \p commandPool.
     * @param commandPool Command pool to allocate command buffers.
     * @param level Command buffer level (default: primary).
     * @return Array of \p N command buffers.
     * @throw vk::Result if failed to allocate command buffers.
     */
    export template <std::size_t N>
    [[nodiscard]] std::array<VULKAN_HPP_NAMESPACE::CommandBuffer, N> allocateCommandBuffers(
        VULKAN_HPP_NAMESPACE::Device device,
        VULKAN_HPP_NAMESPACE::CommandPool commandPool,
        VULKAN_HPP_NAMESPACE::CommandBufferLevel level = {}
    ) {
        std::array<VULKAN_HPP_NAMESPACE::CommandBuffer, N> commandBuffers;
        const VULKAN_HPP_NAMESPACE::Result result = device.allocateCommandBuffers(
            unsafeAddress(VULKAN_HPP_NAMESPACE::CommandBufferAllocateInfo { commandPool, level, N }),
            commandBuffers.data());

        if (result != VULKAN_HPP_NAMESPACE::Result::eSuccess) {
            throw result;
        }
        return commandBuffers;
    }

    export template <std::invocable<VULKAN_HPP_NAMESPACE::CommandBuffer> F>
    struct ExecutionInfo {
        F commandRecorder;
        VULKAN_HPP_NAMESPACE::CommandPool commandPool;
        VULKAN_HPP_NAMESPACE::Queue queue;
        std::optional<std::uint64_t> signalValue { 0ULL };
    };

    /**
     * @brief Allocate a command buffer from \p commandPool, record commands, and submit it to \p queue.
     *
     * The allocated command buffer will automatically begin before and ended after the command buffer recording.
     * If \p fence is null handle, no explicit synchronization is performed and caller has the responsibility to synchronize the command buffer execution ends (such like <tt>queue.waitIdle()</tt>).
     * Otherwise, the function will wait until the command buffer execution ends.
     *
     * @tparam F Function that accepts the command buffer and records commands to it.
     * @param device Vulkan device. Must be same device from \p commandPool.
     * @param commandPool Command pool to allocate a single command buffer.
     * @param queue Queue to submit the allocated command buffer.
     * @param f Function that accepts the command buffer and records commands to it.
     * @param fence Fence to signal when the command buffer is completed, if presented.
     */
    export template <std::invocable<VULKAN_HPP_NAMESPACE::CommandBuffer> F>
        requires std::is_void_v<std::invoke_result_t<F, VULKAN_HPP_NAMESPACE::CommandBuffer>>
    void executeSingleCommand(
        VULKAN_HPP_NAMESPACE::Device device,
        VULKAN_HPP_NAMESPACE::CommandPool commandPool,
        VULKAN_HPP_NAMESPACE::Queue queue,
        F &&f,
        VULKAN_HPP_NAMESPACE::Fence fence = {}
    ) {
        const VULKAN_HPP_NAMESPACE::CommandBuffer commandBuffer = allocateCommandBuffers<1>(device, commandPool)[0];
        commandBuffer.begin({ VULKAN_HPP_NAMESPACE::CommandBufferUsageFlagBits::eOneTimeSubmit });
        std::invoke(FWD(f), commandBuffer);
        commandBuffer.end();
        queue.submit(VULKAN_HPP_NAMESPACE::SubmitInfo {
            {},
            {},
            commandBuffer,
        }, fence);
    }

    /**
     * @copydoc vku::executeSingleCommand<F>(vk::Device, vk::CommandPool, vk::Queue, F&&, vk::Fence)
     */
    export template <std::invocable<VULKAN_HPP_NAMESPACE::CommandBuffer> F>
    [[nodiscard]] std::invoke_result_t<F, VULKAN_HPP_NAMESPACE::CommandBuffer> executeSingleCommand(
        VULKAN_HPP_NAMESPACE::Device device,
        VULKAN_HPP_NAMESPACE::CommandPool commandPool,
        VULKAN_HPP_NAMESPACE::Queue queue,
        F &&f,
        VULKAN_HPP_NAMESPACE::Fence fence = {}
    ) {
        const VULKAN_HPP_NAMESPACE::CommandBuffer commandBuffer = allocateCommandBuffers<1>(device, commandPool)[0];
        commandBuffer.begin({ VULKAN_HPP_NAMESPACE::CommandBufferUsageFlagBits::eOneTimeSubmit });
        auto result = std::invoke(FWD(f), commandBuffer);
        commandBuffer.end();
        queue.submit(VULKAN_HPP_NAMESPACE::SubmitInfo {
            {},
            {},
            commandBuffer,
        }, fence);
        return result;
    }

    export template <typename... ExecutionInfoTuples>
    [[nodiscard]] auto executeHierarchicalCommands(
        const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
        ExecutionInfoTuples &&...executionInfoTuples
    ) -> std::pair<std::vector<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Semaphore>, std::vector<std::uint64_t>> {
        // Count the total required command buffers for each command pool.
        std::unordered_map<VULKAN_HPP_NAMESPACE::CommandPool, std::uint32_t> commandBufferCounts;
        ([&]() {
            apply([&](const auto &...executionInfo) {
                (++commandBufferCounts[executionInfo.commandPool], ...);
            }, executionInfoTuples);
        }(), ...);

        // Make FIFO command buffer queue for each command pools. When all command buffers are submitted, they must be empty.
        std::unordered_map<VULKAN_HPP_NAMESPACE::CommandPool, std::vector<VULKAN_HPP_NAMESPACE::CommandBuffer>> commandBuffersPerPool;
        for (auto [commandPool, commandBufferCount] : commandBufferCounts) {
            commandBuffersPerPool.emplace(commandPool, (*device).allocateCommandBuffers({
                commandPool,
                VULKAN_HPP_NAMESPACE::CommandBufferLevel::ePrimary,
                commandBufferCount,
            }));
        }

        OnDemandCounterStorage timelineSemaphores = makeOnDemandCounterStorage<std::uint64_t>([&]() -> VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Semaphore {
                return { device, VULKAN_HPP_NAMESPACE::StructureChain {
                    VULKAN_HPP_NAMESPACE::SemaphoreCreateInfo{},
                    VULKAN_HPP_NAMESPACE::SemaphoreTypeCreateInfo { VULKAN_HPP_NAMESPACE::SemaphoreType::eTimeline, 0 },
                }.get() };
            });
        std::unordered_map<VULKAN_HPP_NAMESPACE::Semaphore, std::uint64_t> finalSignalSemaphoreValues;

        // Collect the submission command buffers and the signal semaphore by
        // 1) destination queue, 2) wait semaphore value, 3) signal semaphore value (if exist).
        std::map<std::tuple<VULKAN_HPP_NAMESPACE::Queue, std::uint64_t, std::optional<std::uint64_t>>, std::pair<std::vector<VULKAN_HPP_NAMESPACE::CommandBuffer>, VULKAN_HPP_NAMESPACE::Semaphore>> submitInfos;
        std::unordered_multimap<std::uint64_t, VULKAN_HPP_NAMESPACE::Semaphore> waitSemaphoresPerSignalValues;

        apply_with_index([&]<std::size_t Is>(std::integral_constant<std::size_t, Is>, auto &&executionInfos){
            static constexpr std::uint64_t waitSemaphoreValue = Is;
            static constexpr std::uint64_t signalSemaphoreValue = Is + 1;
            apply_by_value([&](auto &&executionInfo) {
                // Get command buffer from FIFO queue and pop it.
                auto &poolCommandBuffers = commandBuffersPerPool[executionInfo.commandPool];
                VULKAN_HPP_NAMESPACE::CommandBuffer commandBuffer = poolCommandBuffers.back();
                poolCommandBuffers.pop_back();

                // Record commands into the commandBuffer by executing executionInfo.commandRecorder.
                commandBuffer.begin({ VULKAN_HPP_NAMESPACE::CommandBufferUsageFlagBits::eOneTimeSubmit });
                std::invoke(FWD(executionInfo.commandRecorder), commandBuffer);
                commandBuffer.end();

                // Push commandBuffer into the corresponding submitInfos entry.
                const std::tuple key {
                    executionInfo.queue,
                    waitSemaphoreValue,
                    executionInfo.signalValue.transform([](auto v) { return v == 0 ? signalSemaphoreValue : v; }),
                };
                auto it = submitInfos.find(key);
                if (it == submitInfos.end()) {
                    VULKAN_HPP_NAMESPACE::Semaphore signalSemaphore = nullptr;
                    if (get<2>(key) /* modified signal value */) {
                        // Register the semaphore for a submission whose waitSemaphoreValue is current's signalSemaphoreValue.
                        signalSemaphore = *timelineSemaphores.at(*get<2>(key));
                        waitSemaphoresPerSignalValues.emplace(signalSemaphoreValue, signalSemaphore);
                    }

                    it = submitInfos.emplace_hint(it, key, std::pair { std::vector<VULKAN_HPP_NAMESPACE::CommandBuffer>{}, signalSemaphore });
                }
                it->second.first/*commandBuffers*/.push_back(commandBuffer);

            }, FWD(executionInfos));
        }, std::forward_as_tuple(FWD(executionInfoTuples)...));

        struct TimelineSemaphoreWaitInfo {
            std::vector<VULKAN_HPP_NAMESPACE::Semaphore> waitSemaphores;
            std::vector<std::uint64_t> waitSemaphoreValues;

            TimelineSemaphoreWaitInfo(std::vector<VULKAN_HPP_NAMESPACE::Semaphore> _waitSemaphores, std::uint64_t waitValue)
                : waitSemaphores { std::move(_waitSemaphores) }
                , waitSemaphoreValues { std::vector(waitSemaphores.size(), waitValue) } { }
        };

        std::unordered_map<VULKAN_HPP_NAMESPACE::Queue, std::vector<VULKAN_HPP_NAMESPACE::SubmitInfo>> submitInfosPerQueue;
        std::vector<TimelineSemaphoreWaitInfo> waitInfos;
        std::forward_list<VULKAN_HPP_NAMESPACE::TimelineSemaphoreSubmitInfo> timelineSemaphoreSubmitInfos;
        // Total dstStageMasks does not exceed the total wait semaphore count (=timelineSemaphores.getValueStorage().size()).
        const std::vector waitDstStageMasks(timelineSemaphores.getValueStorage().size(), VULKAN_HPP_NAMESPACE::Flags { VULKAN_HPP_NAMESPACE::PipelineStageFlagBits::eTopOfPipe });

        for (const auto &[key, value] : submitInfos) {
            const auto &[queue, waitSemaphoreValue, signalSemaphoreValue] = key;
            const auto &[commandBuffers, signalSemaphore] = value;

            constexpr auto make_subrange = []<typename It>(std::pair<It, It> pairs) {
                return std::ranges::subrange(pairs.first, pairs.second);
            };
            const auto &[waitSemaphores, waitSemaphoreValues] = waitInfos.emplace_back(
                make_subrange(waitSemaphoresPerSignalValues.equal_range(waitSemaphoreValue)) | std::views::values | std::ranges::to<std::vector>(),
                waitSemaphoreValue);

            if (signalSemaphoreValue) {
                submitInfosPerQueue[queue].emplace_back(
                    waitSemaphores,
                    unsafeProxy(std::span { waitDstStageMasks }.subspan(0, waitSemaphores.size())),
                    commandBuffers,
                    signalSemaphore,
                    &timelineSemaphoreSubmitInfos.emplace_front(waitSemaphoreValues, *signalSemaphoreValue));

                // Update the final signal semaphore value for current signal semaphore,
                // i.e. update the value to current value if value < current value.
                std::uint64_t &finalSignalValue = finalSignalSemaphoreValues[signalSemaphore];
                finalSignalValue = std::max(finalSignalValue, *signalSemaphoreValue);
            }
            else if (waitSemaphores.empty()) {
                // Don't need to use vk::TimelineSemaphoreSubmitInfo.
                submitInfosPerQueue[queue].push_back({ {}, {}, commandBuffers });
            }
            else {
                submitInfosPerQueue[queue].push_back({
                    waitSemaphores,
                    unsafeProxy(std::span { waitDstStageMasks }.subspan(0, waitSemaphores.size())),
                    commandBuffers,
                    {},
                    &timelineSemaphoreSubmitInfos.emplace_front(waitSemaphoreValues),
                });
            }
        }

        for (const auto &[queue, submitInfos] : submitInfosPerQueue) {
            queue.submit(submitInfos);
        }

        std::pair<std::vector<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Semaphore>, std::vector<std::uint64_t>> result;
        for (VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Semaphore &timelineSemaphore : timelineSemaphores.getValueStorage()) {
            result.second.push_back(finalSignalSemaphoreValues[*timelineSemaphore]);
            result.first.push_back(std::move(timelineSemaphore));
        }
        return result;
    }
}