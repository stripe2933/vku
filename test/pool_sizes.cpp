#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <array>
#include <span>
#include <tuple>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

#ifdef VKU_USE_STD_MODULE
import std;
#endif
import vku;

struct QueueFamilies {
    std::uint32_t compute;

    explicit QueueFamilies(vk::PhysicalDevice physicalDevice)
        : compute { vku::getComputeQueueFamily(physicalDevice.getQueueFamilyProperties()).value() } { }
};

struct Queues {
    Queues(vk::Device, const QueueFamilies&) { }

    [[nodiscard]] static auto getCreateInfos(vk::PhysicalDevice, const QueueFamilies &queueFamilies) noexcept
#ifdef _MSC_VER
        -> vku::RefHolder<std::array<vk::DeviceQueueCreateInfo, 1>, std::array<float, 1>>
#endif
    {
        return vku::RefHolder {
            [&](std::span<const float> priorities) {
                return std::array {
                    vk::DeviceQueueCreateInfo {
                        {},
                        queueFamilies.compute,
                        priorities,
                    },
                };
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
                vk::KHRMaintenance1ExtensionName,
#if __APPLE__
                vk::KHRPortabilitySubsetExtensionName,
#endif
            },
        } } { }
};

int main() {
#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
    VULKAN_HPP_DEFAULT_DISPATCHER.init();
#endif

    const vk::raii::Context context;
    const vk::raii::Instance instance { context, vk::InstanceCreateInfo {
#if __APPLE__
        vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
#else
        {},
#endif
        vku::unsafeAddress(vk::ApplicationInfo {
            "vku_test_pool_sizes", 0,
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

    const vku::DescriptorSetLayout<
        vk::DescriptorType::eSampler,
        vk::DescriptorType::eCombinedImageSampler,
        vk::DescriptorType::eSampledImage,
        vk::DescriptorType::eStorageImage,
        vk::DescriptorType::eUniformBuffer,
        vk::DescriptorType::eStorageBuffer> allDescriptorSetLayout { gpu.device, vk::DescriptorSetLayoutCreateInfo {
        {},
        vku::unsafeProxy({
            vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eSampler, 1, vk::ShaderStageFlagBits::eCompute },
            vk::DescriptorSetLayoutBinding { 1, vk::DescriptorType::eCombinedImageSampler, 2, vk::ShaderStageFlagBits::eCompute },
            vk::DescriptorSetLayoutBinding { 2, vk::DescriptorType::eSampledImage, 3, vk::ShaderStageFlagBits::eCompute },
            vk::DescriptorSetLayoutBinding { 3, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute },
            vk::DescriptorSetLayoutBinding { 4, vk::DescriptorType::eUniformBuffer, 2, vk::ShaderStageFlagBits::eCompute },
            vk::DescriptorSetLayoutBinding { 5, vk::DescriptorType::eStorageBuffer, 3, vk::ShaderStageFlagBits::eCompute },
        }),
    } };
    const std::tuple singleDescriptorSetLayouts {
        vku::DescriptorSetLayout<vk::DescriptorType::eSampler> { gpu.device, vk::DescriptorSetLayoutCreateInfo {
            {},
            vku::unsafeProxy(vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eSampler, 1, vk::ShaderStageFlagBits::eCompute }),
        } },
        vku::DescriptorSetLayout<vk::DescriptorType::eCombinedImageSampler> { gpu.device, vk::DescriptorSetLayoutCreateInfo {
            {},
            vku::unsafeProxy(vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute }),
        } },
        vku::DescriptorSetLayout<vk::DescriptorType::eSampledImage> { gpu.device, vk::DescriptorSetLayoutCreateInfo {
            {},
            vku::unsafeProxy(vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eSampledImage, 1, vk::ShaderStageFlagBits::eCompute }),
        } },
        vku::DescriptorSetLayout<vk::DescriptorType::eStorageImage> { gpu.device, vk::DescriptorSetLayoutCreateInfo {
            {},
            vku::unsafeProxy(vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute }),
        } },
        vku::DescriptorSetLayout<vk::DescriptorType::eUniformBuffer> { gpu.device, vk::DescriptorSetLayoutCreateInfo {
            {},
            vku::unsafeProxy(vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute }),
        } },
        vku::DescriptorSetLayout<vk::DescriptorType::eStorageBuffer> { gpu.device, vk::DescriptorSetLayoutCreateInfo {
            {},
            vku::unsafeProxy(vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute }),
        } },
    };

    const vk::raii::DescriptorPool descriptorPool {
        gpu.device,
        (allDescriptorSetLayout.getPoolSize() * 5
            + std::apply([](const auto &...layouts) { return getPoolSizes(layouts...); }, singleDescriptorSetLayouts))
            .getDescriptorPoolCreateInfo(),
    };
    std::ignore = vku::allocateDescriptorSets(*gpu.device, *descriptorPool, std::tie(
        allDescriptorSetLayout, allDescriptorSetLayout, allDescriptorSetLayout));
    std::ignore = vku::allocateDescriptorSets(*gpu.device, *descriptorPool, std::tie(
        allDescriptorSetLayout, allDescriptorSetLayout));
    std::ignore = vku::allocateDescriptorSets(*gpu.device, *descriptorPool, singleDescriptorSetLayouts);

    // Now it is expected that descriptorPool is exhausted. All types of descriptor set allocation is expected to cause an error.
    try {
        std::ignore = vku::allocateDescriptorSets(*gpu.device, *descriptorPool, std::tie(get<0>(singleDescriptorSetLayouts)));
        return 1;
    }
    catch (vk::OutOfPoolMemoryError) { }
    try {
        std::ignore = vku::allocateDescriptorSets(*gpu.device, *descriptorPool, std::tie(get<1>(singleDescriptorSetLayouts)));
        return 1;
    }
    catch (vk::OutOfPoolMemoryError) { }
    try {
        std::ignore = vku::allocateDescriptorSets(*gpu.device, *descriptorPool, std::tie(get<2>(singleDescriptorSetLayouts)));
        return 1;
    }
    catch (vk::OutOfPoolMemoryError) { }
    try {
        std::ignore = vku::allocateDescriptorSets(*gpu.device, *descriptorPool, std::tie(get<3>(singleDescriptorSetLayouts)));
        return 1;
    }
    catch (vk::OutOfPoolMemoryError) { }
    try {
        std::ignore = vku::allocateDescriptorSets(*gpu.device, *descriptorPool, std::tie(get<4>(singleDescriptorSetLayouts)));
        return 1;
    }
    catch (vk::OutOfPoolMemoryError) { }
    try {
        std::ignore = vku::allocateDescriptorSets(*gpu.device, *descriptorPool, std::tie(get<5>(singleDescriptorSetLayouts)));
        return 1;
    }
    catch (vk::OutOfPoolMemoryError) { }
}