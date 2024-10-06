#include <cassert>
#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <array>
#include <span>
#include <tuple>
#ifdef _MSC_VER
#include <format>
#endif
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

#ifdef VKU_USE_STD_MODULE
import std;
#endif
import vku;

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

struct QueueFamilies {
    std::uint32_t compute;

    explicit QueueFamilies(vk::PhysicalDevice physicalDevice)
        : compute { vku::getComputeQueueFamily(physicalDevice.getQueueFamilyProperties()).value() } { }
};

struct Queues {
    vk::Queue compute;

    Queues(vk::Device device, const QueueFamilies &queueFamilies)
        : compute { device.getQueue(queueFamilies.compute, 0) } { }

    [[nodiscard]] static auto getCreateInfos(vk::PhysicalDevice, const QueueFamilies &queueFamilies) noexcept -> vku::RefHolder<vk::DeviceQueueCreateInfo> {
        return {
            [&]() {
                static constexpr float priority = 1.f;
                return vk::DeviceQueueCreateInfo {
                    {},
                    queueFamilies.compute,
                    vk::ArrayProxyNoTemporaries<const float>(priority),
                };
            },
        };
    }
};

struct Gpu : vku::Gpu<QueueFamilies, Queues> {
    explicit Gpu(const vk::raii::Instance &instance [[clang::lifetimebound]])
        : vku::Gpu<QueueFamilies, Queues> { instance, vku::Gpu<QueueFamilies, Queues>::Config {
            .verbose = true,
#if __APPLE__
            .deviceExtensions = {
                vk::KHRPortabilitySubsetExtensionName,
            },
#endif
        } } { }
};

struct BufferFillComputer {
    vku::DescriptorSetLayout<vk::DescriptorType::eStorageBuffer> descriptorSetLayout;
    vk::raii::PipelineLayout pipelineLayout;
    vk::raii::Pipeline pipeline;

    explicit BufferFillComputer(const vk::raii::Device &device [[clang::lifetimebound]], std::uint32_t value)
        : descriptorSetLayout { device, {
            {},
            vku::unsafeProxy(vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute }),
        } }
        , pipelineLayout { device, vk::PipelineLayoutCreateInfo {
            {},
            *descriptorSetLayout,
        } }
        , pipeline { device, nullptr, vk::ComputePipelineCreateInfo {
            {},
            createPipelineStages(
                device,
                vku::Shader {
                    COMPILED_SHADER_DIR "/buffer_fill.comp.spv",
                    vk::ShaderStageFlagBits::eCompute,
                    // --------------------
                    // MAIN CODE TO TEST!
                    // --------------------
                    vk::SpecializationInfo {
                        vku::unsafeProxy(vk::SpecializationMapEntry { 0, 0, sizeof(value) }),
                        vku::unsafeProxy(value),
                    },
                }).get()[0],
            *pipelineLayout,
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
            "vku_test_specialization_constant", 0,
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

    const vku::MappedBuffer buffer { gpu.allocator, vk::BufferCreateInfo {
        {},
        sizeof(std::uint32_t),
        vk::BufferUsageFlagBits::eStorageBuffer,
    }, vku::allocation::hostRead };

    const BufferFillComputer bufferFillComputer { gpu.device, 0xC3C3C3C3 };

    const vk::raii::DescriptorPool descriptorPool { gpu.device, getPoolSizes(bufferFillComputer.descriptorSetLayout).getDescriptorPoolCreateInfo() };
    const auto [descriptorSet] = vku::allocateDescriptorSets(*gpu.device, *descriptorPool, std::tie(bufferFillComputer.descriptorSetLayout));
    gpu.device.updateDescriptorSets(
        descriptorSet.getWriteOne<0>({ buffer, 0, vk::WholeSize }),
        {});

    const vk::raii::CommandPool commandPool { gpu.device, vk::CommandPoolCreateInfo { {}, gpu.queueFamilies.compute } };
    vku::executeSingleCommand(*gpu.device, *commandPool, gpu.queues.compute, [&](vk::CommandBuffer cb) {
        cb.bindPipeline(vk::PipelineBindPoint::eCompute, *bufferFillComputer.pipeline);
        cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *bufferFillComputer.pipelineLayout, 0, descriptorSet, {});
        cb.dispatch(1, 1, 1);
    });
    gpu.queues.compute.waitIdle();

    assert(buffer.asValue<std::uint32_t>() == 0xC3C3C3C3);
}