#include <cassert>

#include <vulkan/vulkan_hpp_macros.hpp>

import std;
import vku;

class QueueFamilies {
public:
    std::uint32_t compute;
    std::uint32_t graphics;
    std::uint32_t transfer;

    explicit QueueFamilies(vk::PhysicalDevice physicalDevice)
        : QueueFamilies { physicalDevice.getQueueFamilyProperties() } { }

private:
    explicit QueueFamilies(std::span<const vk::QueueFamilyProperties> queueFamilyProperties)
        : compute { vku::getComputeSpecializedQueueFamily(queueFamilyProperties)
            .or_else([&] {
                return vku::getComputeQueueFamily(queueFamilyProperties);
            })
            .value() }
        , graphics { vku::getGraphicsQueueFamily(queueFamilyProperties).value() }
        , transfer { vku::getTransferSpecializedQueueFamily(queueFamilyProperties).value_or(compute) } { }
};

struct Queues {
    vk::Queue compute;
    vk::Queue graphics;
    vk::Queue transfer;

    Queues(vk::Device device, const QueueFamilies &queueFamilies)
        : compute { device.getQueue(queueFamilies.compute, 0) }
        , graphics { device.getQueue(queueFamilies.graphics, 0) }
        , transfer { device.getQueue(queueFamilies.transfer, 0) } { }

    [[nodiscard]] static auto getCreateInfos(vk::PhysicalDevice, const QueueFamilies &queueFamilies) noexcept {
        return vku::RefHolder {
            [&](std::span<const float> priorities) {
                std::vector uniqueIndices { queueFamilies.compute, queueFamilies.graphics, queueFamilies.transfer };
                const auto [begin, end] = std::ranges::unique(uniqueIndices);
                uniqueIndices.erase(begin, end);

                return uniqueIndices
                    | std::views::transform([&](std::uint32_t queueFamilyIndex) {
                        return vk::DeviceQueueCreateInfo {
                            {},
                            queueFamilyIndex,
                            priorities,
                        };
                    })
                    | std::ranges::to<std::vector>();
            },
            std::array { 1.f },
        };
    }
};

struct Gpu : vku::Gpu<QueueFamilies, Queues> {
    explicit Gpu(const vk::raii::Instance &instance [[clang::lifetimebound]])
        : vku::Gpu<QueueFamilies, Queues> { instance, vku::Gpu<QueueFamilies, Queues>::Config {
            .verbose = true,
            .deviceExtensions = {
                vk::KHRTimelineSemaphoreExtensionName,
#if __APPLE__
                vk::KHRPortabilitySubsetExtensionName,
#endif
            },
            .devicePNexts = std::tuple {
                vk::PhysicalDeviceTimelineSemaphoreFeatures { true },
            },
            .apiVersion = vk::makeApiVersion(0, 1, 0, 0),
        } } { }
};

int main() {
    const vk::raii::Context context;
    const vk::raii::Instance instance { context, vk::InstanceCreateInfo {
#if __APPLE__
        vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
#else
        {},
#endif
        vku::unsafeAddress(vk::ApplicationInfo {
            "vku_test_execute_hierarchical_commands", 0,
            {}, 0,
            vk::makeApiVersion(0, 1, 0, 0),
        }),
        {},
#if __APPLE__
        vku::unsafeProxy({
            vk::KHRGetPhysicalDeviceProperties2ExtensionName,
            vk::KHRPortabilityEnumerationExtensionName,
        }),
#endif
    } };
    const Gpu gpu { instance };

    std::vector<vku::MappedBuffer> buffers;
    std::generate_n(back_inserter(buffers), 5, [&]() -> vku::MappedBuffer {
        return { gpu.allocator, vk::BufferCreateInfo {
            {},
            sizeof(std::uint32_t),
            vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
        } };
    });

    const vk::raii::CommandPool computeCommandPool { gpu.device, vk::CommandPoolCreateInfo { {}, gpu.queueFamilies.compute } };
    const vk::raii::CommandPool graphicsCommandPool { gpu.device, vk::CommandPoolCreateInfo { {}, gpu.queueFamilies.graphics } };
    const vk::raii::CommandPool transferCommandPool { gpu.device, vk::CommandPoolCreateInfo { {}, gpu.queueFamilies.transfer } };
    const auto [timelineSemaphores, waitValues] = vku::executeHierarchicalCommands(
        gpu.device,
        std::forward_as_tuple(
            // buffers[0] = 0xC8C8C8C8
            vku::ExecutionInfo { [&](vk::CommandBuffer cb) {
                cb.fillBuffer(buffers[0], 0, sizeof(std::uint32_t), 0xC8C8C8C8);
            }, *computeCommandPool, gpu.queues.compute },
            // buffers[2] = 0xD3D3D3D3
            vku::ExecutionInfo { [&](vk::CommandBuffer cb) {
                cb.fillBuffer(buffers[2], 0, sizeof(std::uint32_t), 0xD3D3D3D3);
            }, *graphicsCommandPool, gpu.queues.graphics, 2 }),
        std::forward_as_tuple(
            // buffers[0] = 0xC8C8C8C8 -> buffers[1]
            vku::ExecutionInfo { [&](vk::CommandBuffer cb) {
                cb.copyBuffer(buffers[0], buffers[1], vk::BufferCopy { 0, 0, sizeof(std::uint32_t) });
            }, *computeCommandPool, gpu.queues.compute },
            // buffers[4] = 0xE4E4E4E4
            vku::ExecutionInfo { [&](vk::CommandBuffer cb) {
                cb.fillBuffer(buffers[4], 0, sizeof(std::uint32_t), 0xE4E4E4E4);
            }, *graphicsCommandPool, gpu.queues.graphics }),
        std::forward_as_tuple(
            // buffers[2] = 0xD3D3D3D3 -> buffers[3]
            vku::ExecutionInfo { [&](vk::CommandBuffer cb) {
                cb.copyBuffer(buffers[2], buffers[3], vk::BufferCopy { 0, 0, sizeof(std::uint32_t) });
            }, *computeCommandPool, gpu.queues.compute },
            // buffers[4] = 0xE4E4E4E4 -> buffers[0]
            vku::ExecutionInfo { [&](vk::CommandBuffer cb) {
                cb.copyBuffer(buffers[4], buffers[0], vk::BufferCopy { 0, 0, sizeof(std::uint32_t) });
            }, *transferCommandPool, gpu.queues.transfer }));

    const vk::Result waitResult = gpu.device.waitSemaphores({
        {},
        vku::unsafeProxy(timelineSemaphores | std::views::transform([](const auto &x) { return *x; }) | std::ranges::to<std::vector>()),
        waitValues
    }, ~0ULL);
    if (waitResult != vk::Result::eSuccess) {
        throw std::runtime_error { "Failed to wait the semaphores!" };
    }

    assert(buffers[0].asValue<std::uint32_t>() == 0xE4E4E4E4);
    assert(buffers[1].asValue<std::uint32_t>() == 0xC8C8C8C8);
    assert(buffers[2].asValue<std::uint32_t>() == 0xD3D3D3D3);
    assert(buffers[3].asValue<std::uint32_t>() == 0xD3D3D3D3);
    assert(buffers[4].asValue<std::uint32_t>() == 0xE4E4E4E4);
}