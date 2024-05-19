#pragma once

#include <functional>

#include <vulkan/vulkan_raii.hpp>

#include "details/concepts.hpp"

#include "details/macros.hpp"

namespace vku {
    auto executeSingleCommand(
        vk::Device device,
        vk::CommandPool commandPool,
        vk::Queue queue,
        details::invocable_r<void, vk::CommandBuffer> auto &&f,
        vk::Fence fence = {}
    ) -> void {
        const vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(vk::CommandBufferAllocateInfo {
            commandPool,
            vk::CommandBufferLevel::ePrimary,
            1,
        })[0];
        commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        std::invoke(VKU_FWD(f), commandBuffer);
        commandBuffer.end();
        queue.submit(vk::SubmitInfo {
            {},
            {},
            commandBuffer,
        }, fence);
    }

    template <std::invocable<vk::CommandBuffer> F>
    [[nodiscard]] auto executeSingleCommand(
        vk::Device device,
        vk::CommandPool commandPool,
        vk::Queue queue,
        F &&f,
        vk::Fence fence = {}
    ) -> std::invoke_result_t<F, vk::CommandBuffer> {
        const vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(vk::CommandBufferAllocateInfo {
            commandPool,
            vk::CommandBufferLevel::ePrimary,
            1,
        })[0];
        commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        auto result = std::invoke(VKU_FWD(f), commandBuffer);
        commandBuffer.end();
        queue.submit(vk::SubmitInfo {
            {},
            {},
            commandBuffer,
        }, fence);
        return result;
    }
}