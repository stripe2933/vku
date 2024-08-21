module;

#include <version>
#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <algorithm>
#include <concepts>
#include <forward_list>
#include <functional>
#include <map>
#include <optional>
#include <queue>
#include <ranges>
#include <span>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:commands;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vulkan_hpp;
import :details.container.OnDemandCounterStorage;
import :details.tuple;
import :utils;

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

#ifdef _MSC_VER
template <>
struct std::hash<VULKAN_HPP_NAMESPACE::CommandPool> {
    size_t operator()( VULKAN_HPP_NAMESPACE::CommandPool const & commandPool ) const VULKAN_HPP_NOEXCEPT {
        return hash<VULKAN_HPP_NAMESPACE::CommandPool::CType>{}( static_cast<VULKAN_HPP_NAMESPACE::CommandPool::CType>( commandPool ) );
    }
};

template <>
struct std::hash<VULKAN_HPP_NAMESPACE::Queue> {
    size_t operator()( VULKAN_HPP_NAMESPACE::Queue const & queue ) const VULKAN_HPP_NOEXCEPT {
        return hash<VULKAN_HPP_NAMESPACE::Queue::CType>{}( static_cast<VULKAN_HPP_NAMESPACE::Queue::CType>( queue ) );
    }
};

template <>
struct std::hash<VULKAN_HPP_NAMESPACE::Semaphore> {
    size_t operator()( VULKAN_HPP_NAMESPACE::Semaphore const & semaphore ) const VULKAN_HPP_NOEXCEPT {
        return hash<VULKAN_HPP_NAMESPACE::Semaphore::CType>{}( static_cast<VULKAN_HPP_NAMESPACE::Semaphore::CType>( semaphore ) );
    }
};
#endif

namespace vku {
    export template <std::invocable<vk::CommandBuffer> F>
    struct ExecutionInfo {
        F commandRecorder;
        vk::CommandPool commandPool;
        vk::Queue queue;
        std::optional<std::uint64_t> signalValue { 0ULL };
    };

    export template <std::invocable<vk::CommandBuffer> F>
    auto executeSingleCommand(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, F &&f, vk::Fence fence = {}) -> void
        requires std::is_void_v<std::invoke_result_t<F, vk::CommandBuffer>>;

    export template <std::invocable<vk::CommandBuffer> F>
    [[nodiscard]] auto executeSingleCommand(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, F &&f, vk::Fence fence = {}) -> std::invoke_result_t<F, vk::CommandBuffer>;

    export template <typename... ExecutionInfoTuples>
    [[nodiscard]] auto executeHierarchicalCommands(
        const vk::raii::Device &device,
        ExecutionInfoTuples &&...executionInfoTuples
    ) -> std::pair<std::vector<vk::raii::Semaphore>, std::vector<std::uint64_t>> {
        // Count the total required command buffers for each command pool.
        std::unordered_map<vk::CommandPool, std::uint32_t> commandBufferCounts;
        ([&]() {
            apply([&](const auto &...executionInfo) {
                (++commandBufferCounts[executionInfo.commandPool], ...);
            }, executionInfoTuples);
        }(), ...);

        // Make FIFO command buffer queue for each command pools. When all command buffers are submitted, they must be empty.
        std::unordered_map<vk::CommandPool, std::vector<vk::CommandBuffer>> commandBuffersPerPool;
        for (auto [commandPool, commandBufferCount] : commandBufferCounts) {
            commandBuffersPerPool.emplace(commandPool, (*device).allocateCommandBuffers({
                commandPool,
                vk::CommandBufferLevel::ePrimary,
                commandBufferCount,
            }));
        }

        container::OnDemandCounterStorage timelineSemaphores
            = container::makeOnDemandCounterStorage<std::uint64_t>([&]() -> vk::raii::Semaphore {
                return { device, vk::StructureChain {
                    vk::SemaphoreCreateInfo{},
                    vk::SemaphoreTypeCreateInfo { vk::SemaphoreType::eTimeline, 0 },
                }.get() };
            });
        std::unordered_map<vk::Semaphore, std::uint64_t> finalSignalSemaphoreValues;

        // Collect the submission command buffers and the signal semaphore by
        // 1) destination queue, 2) wait semaphore value, 3) signal semaphore value (if exist).
        std::map<std::tuple<vk::Queue, std::uint64_t, std::optional<std::uint64_t>>, std::pair<std::vector<vk::CommandBuffer>, vk::Semaphore>> submitInfos;
        std::unordered_multimap<std::uint64_t, vk::Semaphore> waitSemaphoresPerSignalValues;

        apply_with_index([&]<std::size_t Is>(std::integral_constant<std::size_t, Is>, auto &&executionInfos){
            static constexpr std::uint64_t waitSemaphoreValue = Is;
            static constexpr std::uint64_t signalSemaphoreValue = Is + 1;
            apply_by_value([&](auto &&executionInfo) {
                // Get command buffer from FIFO queue and pop it.
                auto &poolCommandBuffers = commandBuffersPerPool[executionInfo.commandPool];
                vk::CommandBuffer commandBuffer = poolCommandBuffers.back();
                poolCommandBuffers.pop_back();

                // Record commands into the commandBuffer by executing executionInfo.commandRecorder.
                commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
                executionInfo.commandRecorder(commandBuffer);
                commandBuffer.end();

                // Push commandBuffer into the corresponding submitInfos entry.
                const std::tuple key {
                    executionInfo.queue,
                    waitSemaphoreValue,
                    executionInfo.signalValue.transform([](auto v) { return v == 0 ? signalSemaphoreValue : v; }),
                };
                auto it = submitInfos.find(key);
                if (it == submitInfos.end()) {
                    vk::Semaphore signalSemaphore = nullptr;
                    if (get<2>(key) /* modified signal value */) {
                        // Register the semaphore for a submission whose waitSemaphoreValue is current's signalSemaphoreValue.
                        signalSemaphore = *timelineSemaphores.at(*get<2>(key));
                        waitSemaphoresPerSignalValues.emplace(signalSemaphoreValue, signalSemaphore);
                    }

                    it = submitInfos.emplace_hint(it, key, std::pair { std::vector<vk::CommandBuffer>{}, signalSemaphore });
                }
                it->second.first/*commandBuffers*/.push_back(commandBuffer);

            }, FWD(executionInfos));
        }, std::forward_as_tuple(FWD(executionInfoTuples)...));

        struct TimelineSemaphoreWaitInfo {
            std::vector<vk::Semaphore> waitSemaphores;
            std::vector<std::uint64_t> waitSemaphoreValues;

            TimelineSemaphoreWaitInfo(std::vector<vk::Semaphore> _waitSemaphores, std::uint64_t waitValue)
                : waitSemaphores { std::move(_waitSemaphores) }
                , waitSemaphoreValues { std::vector(waitSemaphores.size(), waitValue) } { }
        };

        std::unordered_map<vk::Queue, std::vector<vk::SubmitInfo>> submitInfosPerQueue;
        std::vector<TimelineSemaphoreWaitInfo> waitInfos;
        std::forward_list<vk::TimelineSemaphoreSubmitInfo> timelineSemaphoreSubmitInfos;
        // Total dstStageMasks does not exceed the total wait semaphore count (=timelineSemaphores.getValueStorage().size()).
        const std::vector waitDstStageMasks(timelineSemaphores.getValueStorage().size(), vk::Flags { vk::PipelineStageFlagBits::eTopOfPipe });

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

        std::pair<std::vector<vk::raii::Semaphore>, std::vector<std::uint64_t>> result;
        for (vk::raii::Semaphore &timelineSemaphore : timelineSemaphores.getValueStorage()) {
            result.second.push_back(finalSignalSemaphoreValues[*timelineSemaphore]);
            result.first.push_back(std::move(timelineSemaphore));
        }
        return result;
    }
}

template <std::invocable<vk::CommandBuffer> F>
auto vku::executeSingleCommand(
    vk::Device device,
    vk::CommandPool commandPool,
    vk::Queue queue,
    F &&f,
    vk::Fence fence
) -> void requires std::is_void_v<std::invoke_result_t<F, vk::CommandBuffer>>{
    const vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(vk::CommandBufferAllocateInfo {
        commandPool,
        vk::CommandBufferLevel::ePrimary,
        1,
    })[0];
    commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    std::invoke(FWD(f), commandBuffer);
    commandBuffer.end();
    queue.submit(vk::SubmitInfo {
        {},
        {},
        commandBuffer,
    }, fence);
}

template <std::invocable<vk::CommandBuffer> F>
[[nodiscard]] auto vku::executeSingleCommand(
    vk::Device device,
    vk::CommandPool commandPool,
    vk::Queue queue,
    F &&f,
    vk::Fence fence
) -> std::invoke_result_t<F, vk::CommandBuffer> {
    const vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(vk::CommandBufferAllocateInfo {
        commandPool,
        vk::CommandBufferLevel::ePrimary,
        1,
    })[0];
    commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    auto result = std::invoke(FWD(f), commandBuffer);
    commandBuffer.end();
    queue.submit(vk::SubmitInfo {
        {},
        {},
        commandBuffer,
    }, fence);
    return result;
}