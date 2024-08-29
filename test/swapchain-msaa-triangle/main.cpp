#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <algorithm>
#include <array>
#include <format>
#include <ranges>
#include <span>
#include <stdexcept>
#include <tuple>
#include <variant>
#include <vector>
#ifdef _MSC_VER
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
    std::uint32_t graphicsPresent;

    QueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
        : graphicsPresent { vku::getGraphicsPresentQueueFamily(physicalDevice, surface, physicalDevice.getQueueFamilyProperties()).value() } { }
};

struct Queues {
    vk::Queue graphicsPresent;

    Queues(vk::Device device, const QueueFamilies &queueFamilies)
        : graphicsPresent { device.getQueue(queueFamilies.graphicsPresent, 0) } { }

    [[nodiscard]] static auto getCreateInfos(vk::PhysicalDevice, const QueueFamilies &queueFamilies) noexcept
#ifdef _MSC_VER
        -> vku::RefHolder<std::array<vk::DeviceQueueCreateInfo, 1>, std::array<float, 1>>
#endif
    {
        return vku::RefHolder {
            [=](std::span<const float> priorities) {
                return std::array {
                    vk::DeviceQueueCreateInfo {
                        {},
                        queueFamilies.graphicsPresent,
                        priorities,
                    },
                };
            },
            std::array { 1.f },
        };
    }
};

struct Gpu : vku::Gpu<QueueFamilies, Queues> {
    explicit Gpu(const vk::raii::Instance &instance [[clang::lifetimebound]], vk::SurfaceKHR surface)
        : vku::Gpu<QueueFamilies, Queues> { instance, vku::Gpu<QueueFamilies, Queues>::Config<vk::PhysicalDeviceDynamicRenderingFeatures, vk::PhysicalDeviceSynchronization2Features> {
            .verbose = true,
            .deviceExtensions = {
                vk::KHRMultiviewExtensionName,
                vk::KHRMaintenance2ExtensionName,
                vk::KHRCreateRenderpass2ExtensionName,
                vk::KHRDepthStencilResolveExtensionName,
                vk::KHRDynamicRenderingExtensionName,
                vk::KHRSynchronization2ExtensionName,
#if __APPLE__
                vk::KHRPortabilitySubsetExtensionName,
#endif
                vk::KHRSwapchainExtensionName,
            },
            .devicePNexts = std::tuple {
                vk::PhysicalDeviceDynamicRenderingFeatures { true },
                vk::PhysicalDeviceSynchronization2Features { true },
            },
            .queueFamilyGetter = [=](vk::PhysicalDevice physicalDevice) {
                return QueueFamilies { physicalDevice, surface };
            },
            .apiVersion = vk::makeApiVersion(0, 1, 0, 0),
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
            "vku_test_swapchain_msaa_triangle", 0,
            {}, 0,
            vk::makeApiVersion(0, 1, 0, 0),
        }),
        {},
        vku::unsafeProxy({
#if __APPLE__
            vk::KHRGetPhysicalDeviceProperties2ExtensionName,
            vk::KHRPortabilityEnumerationExtensionName,
#endif
            vk::KHRSurfaceExtensionName,
            vk::EXTHeadlessSurfaceExtensionName,
        }),
    } };
    const vk::raii::SurfaceKHR surface = instance.createHeadlessSurfaceEXT(vk::HeadlessSurfaceCreateInfoEXT{});
    const Gpu gpu { instance, *surface };

    const vk::SurfaceCapabilitiesKHR surfaceCapabilities = gpu.physicalDevice.getSurfaceCapabilitiesKHR(*surface);
    const vk::raii::SwapchainKHR swapchain { gpu.device, vk::SwapchainCreateInfoKHR {
        {},
        *surface,
        std::max(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.maxImageCount),
        vk::Format::eB8G8R8A8Srgb,
        vk::ColorSpaceKHR::eSrgbNonlinear,
        { 512, 512 },
        1,
        vk::ImageUsageFlagBits::eColorAttachment,
        vk::SharingMode::eExclusive, {},
        surfaceCapabilities.currentTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        vk::PresentModeKHR::eFifo,
        true,
    } };
    const std::vector swapchainImages = (*gpu.device).getSwapchainImagesKHR(*swapchain);

    vku::MsaaAttachmentGroup attachmentGroup { { 512, 512 }, vk::SampleCountFlagBits::e4 };
    attachmentGroup.addSwapchainAttachment(
        gpu.device,
        attachmentGroup.storeImage(attachmentGroup.createColorImage(gpu.allocator, vk::Format::eB8G8R8A8Srgb)),
        swapchainImages,
        vk::Format::eB8G8R8A8Srgb);
    attachmentGroup.setDepthStencilAttachment(
        gpu.device,
        attachmentGroup.storeImage(attachmentGroup.createDepthStencilImage(gpu.allocator, vk::Format::eD32Sfloat)));

    vk::raii::PipelineLayout pipelineLayout { gpu.device, vk::PipelineLayoutCreateInfo{} };
    vk::raii::Pipeline pipeline { gpu.device, nullptr, vk::StructureChain {
        vku::getDefaultGraphicsPipelineCreateInfo(
            createPipelineStages(
                gpu.device,
                vku::Shader { COMPILED_SHADER_DIR "/triangle.vert.spv", vk::ShaderStageFlagBits::eVertex },
                vku::Shader { COMPILED_SHADER_DIR "/triangle.frag.spv", vk::ShaderStageFlagBits::eFragment }).get(),
            *pipelineLayout, 1, true, vk::SampleCountFlagBits::e4),
        // Specify the attachment formats for dynamic rendering.
        vk::PipelineRenderingCreateInfo {
            {},
            vku::unsafeProxy(vk::Format::eB8G8R8A8Srgb),
            vk::Format::eD32Sfloat,
        },
    }.get() };

    const vk::raii::CommandPool graphicsCommandPool { gpu.device, vk::CommandPoolCreateInfo {
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        gpu.queueFamilies.graphicsPresent
    } };
    const vk::CommandBuffer commandBuffer = (*gpu.device).allocateCommandBuffers(vk::CommandBufferAllocateInfo {
        *graphicsCommandPool,
        vk::CommandBufferLevel::ePrimary,
        1,
    })[0];

    // Change all swapchain image layouts to PresentSrcKHR to avoid Undefined format for srcImageLayout of future
    // pipeline barriers.
    vku::executeSingleCommand(*gpu.device, *graphicsCommandPool, gpu.queues.graphicsPresent, [&](vk::CommandBuffer cb) {
         cb.pipelineBarrier(
             vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe,
             {}, {}, {},
             swapchainImages
                | std::views::transform([](vk::Image swapchainImage) {
                    return vk::ImageMemoryBarrier {
                        {}, {},
                        {}, vk::ImageLayout::ePresentSrcKHR,
                        vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                        swapchainImage, vku::fullSubresourceRange(),
                    };
                })
                | std::ranges::to<std::vector>());
    });
    gpu.queues.graphicsPresent.waitIdle();

    const vk::raii::Semaphore swapchainImageAcquireSemaphore { gpu.device, vk::SemaphoreCreateInfo{} };
    const vk::raii::Semaphore renderFinishedSemaphore { gpu.device, vk::SemaphoreCreateInfo{} };
    const vk::raii::Fence frameFinishedFence { gpu.device, vk::FenceCreateInfo { vk::FenceCreateFlagBits::eSignaled } };

    for (std::uint64_t frame = 0; frame < 10ULL; ++frame) {
        // Wait for the previous frame to be finished.
        if (vk::Result result = gpu.device.waitForFences(*frameFinishedFence, true, ~0ULL); result != vk::Result::eSuccess) {
            throw std::runtime_error { std::format("Failed to wait for the previous frame {} to finish: {}", frame - 1, to_string(result)) };
        }
        gpu.device.resetFences(*frameFinishedFence);

        // Acquire swapchain image.
        const auto [swapchainImageAcquireResult, swapchainImageIndex] = (*gpu.device).acquireNextImageKHR(*swapchain, ~0ULL, *swapchainImageAcquireSemaphore);
        if (swapchainImageAcquireResult != vk::Result::eSuccess) {
            throw std::runtime_error { std::format("Failed to acquire swapchain image: {}", to_string(swapchainImageAcquireResult)) };
        }

        // Record draw commands to commandBuffer.
        commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        // Change attachment layouts for rendering.
        commandBuffer.pipelineBarrier2KHR({
            {}, {}, {},
            vku::unsafeProxy({
                vk::ImageMemoryBarrier2 {
                    {}, {},
                    vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite,
                    {}, vk::ImageLayout::eAttachmentOptimal,
                    vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                    get<vku::SwapchainMsaaAttachment>(attachmentGroup.colorAttachments[0]).image, vku::fullSubresourceRange(),
                },
                vk::ImageMemoryBarrier2 {
                    vk::PipelineStageFlagBits2::eColorAttachmentOutput, {},
                    vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite,
                    vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eAttachmentOptimal,
                    vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                    swapchainImages[swapchainImageIndex], vku::fullSubresourceRange(),
                },
                vk::ImageMemoryBarrier2 {
                    {}, {},
                    vk::PipelineStageFlagBits2::eEarlyFragmentTests, vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                    {}, vk::ImageLayout::eAttachmentOptimal,
                    vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                    attachmentGroup.depthStencilAttachment->image, vku::fullSubresourceRange(vk::ImageAspectFlagBits::eDepth),
                },
            }),
        } DEVICE_DISPATCHER_PARAM_OPT(gpu.device));

        // Begin dynamic rendering with clearing the color attachment by (0, 0, 0, 0) (transparent).
        commandBuffer.beginRenderingKHR(attachmentGroup.getRenderingInfo(
            std::array {
                vku::MsaaAttachmentGroup::ColorAttachmentInfo { vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, { 0.f, 0.f, 0.f, 0.f } },
            },
            vku::MsaaAttachmentGroup::DepthStencilAttachmentInfo { vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, { 1.f, 0U } },
            swapchainImageIndex) DEVICE_DISPATCHER_PARAM_OPT(gpu.device));

        commandBuffer.setViewport(0, vku::toViewport(attachmentGroup.extent));
        commandBuffer.setScissor(0, vk::Rect2D { { 0, 0 }, attachmentGroup.extent });

        // Draw triangle.
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
        commandBuffer.draw(3, 1, 0, 0);

        // End dynamic rendering.
        commandBuffer.endRenderingKHR(DEVICE_DISPATCHER_PARAM(gpu.device));

        // Change attachment layout to PresentSrcKHR.
        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe,
            {}, {}, {},
            vk::ImageMemoryBarrier {
                vk::AccessFlagBits::eColorAttachmentWrite, {},
                vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
                vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                swapchainImages[swapchainImageIndex], vku::fullSubresourceRange(),
            });

        commandBuffer.end();

        // Submit commandBuffer to the queue.
        gpu.queues.graphicsPresent.submit(vk::SubmitInfo {
            *swapchainImageAcquireSemaphore,
            vku::unsafeProxy(vk::Flags { vk::PipelineStageFlagBits::eColorAttachmentOutput }),
            commandBuffer,
            *renderFinishedSemaphore,
        }, *frameFinishedFence);

        // Present swapchain image.
        if (vk::Result result = gpu.queues.graphicsPresent.presentKHR({ *renderFinishedSemaphore, *swapchain, swapchainImageIndex });
            result != vk::Result::eSuccess) {
            throw std::runtime_error { std::format("Failed to present the swapchain image {}: {}", swapchainImageIndex, to_string(result)) };
        }
    }

    // Wait until device gets idle.
    gpu.device.waitIdle();
}