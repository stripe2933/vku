#include <extlibs/ranges.hpp>
#include <vku/Instance.hpp>
#include <vku/Gpu.hpp>

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

int main(){
    // Create vk::raii::Context and vk::raii::Instance.
    // Validation layer automatically enabled in Debug mode (when NDEBUG macro is not defined).
    // Proper instance extensions for MoltenVK environment (macOS) are automatically added.
    // Dynamic dispatcher initialization is automatically done, when VULKAN_HPP_DISPATCH_LOADER_DYNAMIC macro is defined as 1.
    const vku::Instance instance {
        vk::ApplicationInfo {
            "Compute Device", 0,
            {}, 0,
            vk::makeApiVersion(0, 1, 0, 0),
        },
        // Can pass various configurations (instance layer/extensions, pNexts...), but we'll use default.
        // vku::Instance::Config { ... }
    };

    // Create vk::raii::PhysicalDevice, vk::raii::Device and its queues.
    const vku::Gpu<QueueFamilyIndices, Queues> gpu {
        instance.instance,
        // Can pass various configurations (device extensions, physical device features, physical device selecting
        // strategy...), but we'll use default.
    };

    return 0;
}