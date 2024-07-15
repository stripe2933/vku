module;

#ifndef VKU_USE_STD_MODULE
#include <concepts>
#include <functional>
#include <type_traits>
#endif

export module vku:commands;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vulkan_hpp;

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

namespace vku {
    export template <std::invocable<vk::CommandBuffer> F>
    auto executeSingleCommand(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, F &&f, vk::Fence fence = {}) -> void
        requires std::is_void_v<std::invoke_result_t<F, vk::CommandBuffer>>;

    export template <std::invocable<vk::CommandBuffer> F>
    [[nodiscard]] auto executeSingleCommand(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, F &&f, vk::Fence fence = {}) -> std::invoke_result_t<F, vk::CommandBuffer>;
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