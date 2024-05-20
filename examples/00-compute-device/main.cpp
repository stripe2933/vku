#include <extlibs/ranges.hpp>
#include <vku/Allocator.hpp>
#include <vku/buffers.hpp>
#include <vku/commands.hpp>
#include <vku/Instance.hpp>
#include <vku/Gpu.hpp>

#include "pipelines/MultiplyComputer.hpp"

struct QueueFamilyIndices {
    std::uint32_t compute;

    explicit QueueFamilyIndices(
        vk::PhysicalDevice physicalDevice
    ) {
        for (auto [queueFamilyIndex, properties] : physicalDevice.getQueueFamilyProperties() | ranges::views::enumerate) {
            if (properties.queueFlags & vk::QueueFlagBits::eCompute) {
                compute = queueFamilyIndex;
                return;
            }
        }

        // Throw error when failing to find a required queue families.
        // throw std::runtime_error { "No compute queue family found." };
        // ... however Vulkan guarantees that there is at least one queue family that supports compute, therefore:
        std::unreachable();
    }
};

struct Queues {
    vk::Queue compute;

    Queues(
        vk::Device device,
        const QueueFamilyIndices &queueFamilyIndices
    ) : compute { device.getQueue(queueFamilyIndices.compute, 0) } { }

    [[nodiscard]] static auto getDeviceQueueCreateInfos(
        const QueueFamilyIndices &queueFamilyIndices
    ) -> std::array<vk::DeviceQueueCreateInfo, 1> /* return type must be a range of vk::DeviceQueueCreateInfo */ {
        static constexpr std::array queuePriorities { 1.f };
        return { vk::DeviceQueueCreateInfo { {}, queueFamilyIndices.compute, queuePriorities } };
        // queuePriorities is declared as a static variable to avoid vk::DeviceQueueCreateInfo points to dangling when returned.
    }
};

class MainApp {
public:
    MainApp() = default;

    auto run() const -> void {
        // Create compute pipeline.
        const MultiplyComputer multiplyComputer { gpu.device };

        // Create descriptor pool.
        constexpr vk::DescriptorPoolSize descriptorPoolSize { vk::DescriptorType::eStorageBuffer, 1 };
        const vk::raii::DescriptorPool descriptorPool { gpu.device, vk::DescriptorPoolCreateInfo {
            {},
            1,
            descriptorPoolSize,
        } };

        // Get descriptor sets from descriptor pool.
        const MultiplyComputer::DescriptorSets multiplyComputerSets { *gpu.device, *descriptorPool, multiplyComputer.descriptorSetLayouts };

        // Create buffer for multiplication.
        constexpr auto inputData = std::views::iota(0, 384) | std::views::transform([](int i) {
            return static_cast<float>(i);
        });
        const vku::MappedBuffer buffer {
            allocator,
            std::from_range, inputData,
            vk::BufferUsageFlagBits::eStorageBuffer,
        };

        // Update descriptor sets.
        gpu.device.updateDescriptorSets(
            multiplyComputerSets.getDescriptorWrites0({ buffer, 0, vk::WholeSize }).get(),
            {});

        // Create command pool.
        const vk::raii::CommandPool computeCommandPool { gpu.device, vk::CommandPoolCreateInfo {
            {},
            gpu.queueFamilyIndices.compute,
        } };

        // Dispatch pipeline.
        vku::executeSingleCommand(*gpu.device, *computeCommandPool, gpu.queues.compute, [&](vk::CommandBuffer commandBuffer) {
            multiplyComputer.compute(commandBuffer, multiplyComputerSets, { 384, 2.f });
        });
        gpu.queues.compute.waitIdle();

        // Check if buffer is multiplied by 2.
        const std::span<const float> result = buffer.asRange<float>();
        constexpr auto expect = inputData | std::views::transform([](float x) { return x * 2.f; });
        const auto [it1, it2] = std::ranges::mismatch(result, expect);
        if (it1 != result.end()) {
            std::println(stderr, "Mismatch at {} in buffer: {}", std::distance(result.begin(), it1), *it1);
        }
        if (it2 != expect.end()) {
            std::println(stderr, "Mismatch at {} in expectation: {}", std::distance(expect.begin(), it2), *it2);
        }
    }

private:
    vku::Instance instance = createInstance();
    vku::Gpu<QueueFamilyIndices, Queues> gpu = createGpu();
    vku::Allocator allocator = createAllocator();

    [[nodiscard]] auto createGpu() const -> vku::Gpu<QueueFamilyIndices, Queues> {
        // Create vk::raii::PhysicalDevice, vk::raii::Device and its queues.
        return vku::Gpu<QueueFamilyIndices, Queues> {
            instance.instance,
            // Can pass various configurations (device extensions, physical device features, physical device selecting
            // strategy...), but we'll use default.
        };
    }

    [[nodiscard]] auto createAllocator() const -> vku::Allocator {
        return vku::Allocator { vma::AllocatorCreateInfo {
            {},
            *gpu.physicalDevice,
            *gpu.device,
            {}, {}, {}, {}, {},
            *instance.instance,
            vk::makeApiVersion(0, 1, 0, 0),
        } };
    }

    [[nodiscard]] static auto createInstance() -> vku::Instance {
        // Create vk::raii::Context and vk::raii::Instance.
        // Validation layer automatically enabled in Debug mode (when NDEBUG macro is not defined).
        // Proper instance extensions for MoltenVK environment (macOS) are automatically added.
        // Dynamic dispatcher initialization is automatically done, when VULKAN_HPP_DISPATCH_LOADER_DYNAMIC macro is defined as 1.
        return vku::Instance {
            vk::ApplicationInfo {
                "Compute Device", 0,
                {}, 0,
                vk::makeApiVersion(0, 1, 0, 0),
            },
            // Can pass various configurations (instance layer/extensions, pNexts...), but we'll use default.
            // vku::Instance::Config { ... }
        };
    }
};

int main() {
    MainApp{}.run();
    return 0;
}