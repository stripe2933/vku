#include <set>

#include <extlibs/ranges.hpp>
#include <vku/Instance.hpp>
#include <vku/Gpu.hpp>

struct QueueFamilyIndices {
    std::uint32_t compute;
    std::uint32_t present;

    QueueFamilyIndices(
        vk::PhysicalDevice physicalDevice,
        vk::SurfaceKHR surface
    ) {
        for (std::optional<std::uint32_t> computeIndex, presentIndex;
             auto [queueFamilyIndex, properties] : physicalDevice.getQueueFamilyProperties() | ranges::views::enumerate) {
            if (properties.queueFlags & vk::QueueFlagBits::eCompute) {
                computeIndex = queueFamilyIndex;
            }
            if (physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, surface)) {
                presentIndex = queueFamilyIndex;
            }

            if (computeIndex && presentIndex) {
                compute = *computeIndex;
                present = *presentIndex;
                return;
            }
        }

        throw std::runtime_error { "Physical device does not support required queue families." };
    }
};

struct Queues {
    vk::Queue compute;
    vk::Queue present;

    Queues(
        vk::Device device,
        const QueueFamilyIndices &queueFamilyIndices
    ) : compute { device.getQueue(queueFamilyIndices.compute, 0) },
        present { device.getQueue(queueFamilyIndices.present, 0) } { }

    [[nodiscard]] static auto getDeviceQueueCreateInfos(
        const QueueFamilyIndices &queueFamilyIndices
    ) -> std::vector<vk::DeviceQueueCreateInfo> {
        return std::set { queueFamilyIndices.compute, queueFamilyIndices.present }
            | std::views::transform([](std::uint32_t queueFamilyIndex) {
                static constexpr std::array queuePriorities { 1.f };
                return vk::DeviceQueueCreateInfo { {}, queueFamilyIndex, queuePriorities };
            })
            | std::ranges::to<std::vector>();
    }
};

int main(){
    // Create vk::raii::Context and vk::raii::Instance.
    const vku::Instance instance {
        vk::ApplicationInfo {
            "Compute Device", 0,
            {}, 0,
            vk::makeApiVersion(0, 1, 0, 0),
        },
        vku::Instance::Config {
            .extensions = {
                vk::KHRSurfaceExtensionName, // Required by VK_EXT_headless_surface.
                vk::EXTHeadlessSurfaceExtensionName,
            },
        }
    };

    // Create headless surface.
    vk::raii::SurfaceKHR surface = instance.instance.createHeadlessSurfaceEXT({});

    // Create vk::raii::PhysicalDevice, vk::raii::Device and its queues.
    const vku::Gpu<QueueFamilyIndices, Queues> gpu {
        instance.instance,
        vku::Gpu<QueueFamilyIndices, Queues>::Config {
            // For default, vku::Gpu tests that the physical device has required queue families by passed argument.
            // Our QueueFamilyIndices constructor has two arguments (vk::PhysicalDevice, vk::SurfaceKHR), therefore
            // queueFamilyIndicesGetter have to be manually specified.
            .queueFamilyIndicesGetter = [&surface](vk::PhysicalDevice physicalDevice) {
                return QueueFamilyIndices { physicalDevice, *surface };
            },
        },
    };

    return 0;
}