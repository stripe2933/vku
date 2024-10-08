#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <array>
#include <span>
#include <tuple>
#include <variant>
#ifdef _MSC_VER
#include <forward_list>
#include <string_view>
#endif
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

#ifdef VKU_USE_STD_MODULE
import std;
#endif
import vku;

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#define DEVICE_DISPATCHER_PARAM(device)
#define DEVICE_DISPATCHER_PARAM_OPT(device)
#else
#define DEVICE_DISPATCHER_PARAM(device) *device.getDispatcher()
#define DEVICE_DISPATCHER_PARAM_OPT(device) , DEVICE_DISPATCHER_PARAM(device)
#endif

struct QueueFamilies {
    std::uint32_t graphics;

    explicit QueueFamilies(vk::PhysicalDevice physicalDevice)
        : graphics { vku::getGraphicsQueueFamily(physicalDevice.getQueueFamilyProperties()).value() } { }
};

struct Queues {
    vk::Queue graphics;

    Queues(vk::Device device, const QueueFamilies &queueFamilies)
        : graphics { device.getQueue(queueFamilies.graphics, 0) } { }

    [[nodiscard]] static auto getCreateInfos(vk::PhysicalDevice, const QueueFamilies &queueFamilies) noexcept -> vku::RefHolder<vk::DeviceQueueCreateInfo> {
        return vku::RefHolder {
            [&]() {
                static constexpr float priority = 1.f;
                return vk::DeviceQueueCreateInfo {
                    {},
                    queueFamilies.graphics,
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
            .deviceExtensions = {
                vk::KHRMultiviewExtensionName,
                vk::KHRMaintenance2ExtensionName,
                vk::KHRCreateRenderpass2ExtensionName,
                vk::KHRDepthStencilResolveExtensionName,
                vk::KHRDynamicRenderingExtensionName,
#if __APPLE__
                vk::KHRPortabilitySubsetExtensionName,
#endif
            },
            .devicePNexts = std::tuple {
                vk::PhysicalDeviceDynamicRenderingFeatures { true },
            },
        } } { }
};

int main(){
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
            "vku_test_triangle", 0,
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

    vku::AttachmentGroup attachmentGroup { { 512, 512 } };
    attachmentGroup.addColorAttachment(
        gpu.device,
        attachmentGroup.storeImage(attachmentGroup.createColorImage(gpu.allocator, vk::Format::eR8G8B8A8Unorm)));

    const vk::raii::PipelineLayout pipelineLayout { gpu.device, vk::PipelineLayoutCreateInfo{} };
    const vk::raii::Pipeline pipeline { gpu.device, nullptr, vk::StructureChain {
        vku::getDefaultGraphicsPipelineCreateInfo(
            createPipelineStages(
                gpu.device,
                vku::Shader { COMPILED_SHADER_DIR "/triangle.vert.spv", vk::ShaderStageFlagBits::eVertex },
                vku::Shader { COMPILED_SHADER_DIR "/triangle.frag.spv", vk::ShaderStageFlagBits::eFragment }).get(),
            *pipelineLayout, 1),
        // Specify the attachment formats for dynamic rendering.
        vk::PipelineRenderingCreateInfo {
            {},
            vku::unsafeProxy(vk::Format::eR8G8B8A8Unorm),
        },
    }.get() };

    const vk::raii::CommandPool graphicsCommandPool { gpu.device, vk::CommandPoolCreateInfo {
        {},
        gpu.queueFamilies.graphics
    } };

    // Allocate a command buffer and submit it into the queue immediately.
    vku::executeSingleCommand(*gpu.device, *graphicsCommandPool, gpu.queues.graphics, [&](vk::CommandBuffer cb) {
        // .begin() and .end are called automatically.

        // Change attachment layout to ColorAttachmentOptimal.
        cb.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
            {}, {}, {},
            vk::ImageMemoryBarrier {
                {}, vk::AccessFlagBits::eColorAttachmentWrite,
                {}, vk::ImageLayout::eColorAttachmentOptimal,
                vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                attachmentGroup.getColorAttachment(0).image, vku::fullSubresourceRange(),
            });

        // Begin dynamic rendering with clearing the color attachment by (0, 0, 0, 0) (transparent).
        cb.beginRenderingKHR(attachmentGroup.getRenderingInfo(
            vku::AttachmentGroup::ColorAttachmentInfo { vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, { 0.f, 0.f, 0.f, 0.f } }) DEVICE_DISPATCHER_PARAM_OPT(gpu.device));

        cb.setViewport(0, vku::toViewport(attachmentGroup.extent));
        cb.setScissor(0, vk::Rect2D { { 0, 0 }, attachmentGroup.extent });

        // Draw triangle.
        cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
        cb.draw(3, 1, 0, 0);

        // End dynamic rendering.
        cb.endRenderingKHR(DEVICE_DISPATCHER_PARAM(gpu.device));
    });

    // Note that vku::executeSingleCommand does not wait for the command buffer to be executed, so we need to wait manually.
    gpu.queues.graphics.waitIdle();
}