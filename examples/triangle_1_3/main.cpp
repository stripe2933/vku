#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef VKU_EXAMPLE_USE_MODULE
#include <vulkan/vulkan_hpp_macros.hpp>

import std;
import vku;
#else
#include <cstdint>
#include <algorithm>
#include <array>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include <vku.hpp>
#endif

class Gpu {
public:
    vk::raii::PhysicalDevice physicalDevice;
    std::uint32_t queueFamily;
    vk::raii::Device device;
    vk::Queue queue;

    Gpu(vk::raii::PhysicalDevice &&_physicalDevice, vk::SurfaceKHR surface)
        : physicalDevice { std::move(_physicalDevice) }
        , queueFamily { getQueueFamily(surface) }
        , device { createDevice() }
        , queue { (*device).getQueue(queueFamily, 0) } { }

private:
    [[nodiscard]] std::uint32_t getQueueFamily(vk::SurfaceKHR surface) const {
        // Find queue family that is both capable of graphics operations and presentation.
        for (std::uint32_t index = 0; const vk::QueueFamilyProperties &props : physicalDevice.getQueueFamilyProperties()) {
            if (vku::contains(props.queueFlags, vk::QueueFlagBits::eGraphics) && physicalDevice.getSurfaceSupportKHR(index, surface)) {
                return index;
            }
            ++index;
        }

        throw std::runtime_error { "Failed to find a suitable queue family" };
    }

    [[nodiscard]] vk::raii::Device createDevice() const {
        std::vector extensions {
            vk::KHRSwapchainExtensionName,
        };

    #if __APPLE__
        for (const vk::ExtensionProperties &props : physicalDevice.enumerateDeviceExtensionProperties()) {
            if (static_cast<std::string_view>(props.extensionName) == vk::KHRPortabilitySubsetExtensionName) {
                // This application supports the Vulkan portability subset.
                extensions.push_back(vk::KHRPortabilitySubsetExtensionName);
                break;
            }
        }
    #endif

        vk::raii::Device result { physicalDevice, vk::StructureChain {
            vk::DeviceCreateInfo {
                {},
                vku::lvalue(vk::DeviceQueueCreateInfo {
                    {},
                    queueFamily,
                    vk::ArrayProxyNoTemporaries<const float> { vku::lvalue(1.f) }
                }),
                {},
                extensions,
            },
            vk::PhysicalDeviceFeatures2{},
            vk::PhysicalDeviceVulkan12Features{}
                .setTimelineSemaphore(true),
            vk::PhysicalDeviceVulkan13Features{}
                .setDynamicRendering(true)
                .setSynchronization2(true),
        }.get() };

    #if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
        // Initialize per-device Vulkan function pointers.
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*result);
    #endif

        return result;
    }
};

class TriangleRenderPipeline {
public:
    vk::raii::PipelineLayout pipelineLayout;
    vk::raii::Pipeline pipeline;

    explicit TriangleRenderPipeline(const vk::raii::Device &device)
        : pipelineLayout { device, vk::PipelineLayoutCreateInfo{} }
        , pipeline { device, nullptr, vk::StructureChain {
            vk::GraphicsPipelineCreateInfo {
                {},
                vku::lvalue({
                    vk::PipelineShaderStageCreateInfo {
                        {},
                        vk::ShaderStageFlagBits::eVertex,
                        *vku::lvalue(vk::raii::ShaderModule { device, vk::ShaderModuleCreateInfo {
                            {},
                            vertexShaderCode,
                        } }),
                        "main",
                    },
                    vk::PipelineShaderStageCreateInfo {
                        {},
                        vk::ShaderStageFlagBits::eFragment,
                        *vku::lvalue(vk::raii::ShaderModule { device, vk::ShaderModuleCreateInfo {
                            {},
                            fragmentShaderCode,
                        } }),
                        "main",
                    },
                }),
                &vku::lvalue(vk::PipelineVertexInputStateCreateInfo{}),
                &vku::lvalue(vku::defaultPipelineInputAssemblyState(vk::PrimitiveTopology::eTriangleList)),
                nullptr,
                &vku::lvalue(vk::PipelineViewportStateCreateInfo {
                    {},
                    1, nullptr,
                    1, nullptr,
                }),
                &vku::lvalue(vku::defaultPipelineRasterizationState()),
                &vku::lvalue(vk::PipelineMultisampleStateCreateInfo { {}, vk::SampleCountFlagBits::e1 }),
                nullptr,
                &vku::lvalue(vku::defaultPipelineColorBlendState(1)),
                &vku::lvalue(vk::PipelineDynamicStateCreateInfo {
                    {},
                    vku::lvalue({ vk::DynamicState::eViewport, vk::DynamicState::eScissor }),
                }),
                *pipelineLayout,
            },
            vk::PipelineRenderingCreateInfo {
                {},
                vku::lvalue(vk::Format::eB8G8R8A8Srgb),
            },
        }.get() } { }

private:
    static constexpr std::uint32_t vertexShaderCode[] = {
        #include "shaders/triangle.vert.spv.h"
    };

    static constexpr std::uint32_t fragmentShaderCode[] = {
        #include "shaders/triangle.frag.spv.h"
    };
};

class Swapchain {
public:
    vk::Extent2D extent;
    vk::raii::SwapchainKHR swapchain;
    std::vector<vk::Image> images;
    std::vector<vk::raii::ImageView> imageViews;
    std::vector<vk::raii::Semaphore> imageReadySemaphores;

    Swapchain(
        const vk::raii::Device &device,
        vk::SurfaceKHR surface,
        const vk::Extent2D &extent,
        const vk::SurfaceCapabilitiesKHR &surfaceCapabilities,
        vk::SwapchainKHR oldSwapchain = {}
    ) : extent { extent },
        swapchain { device, vk::SwapchainCreateInfoKHR {
            {},
            surface,
            getMinImageCount(surfaceCapabilities),
            vk::Format::eB8G8R8A8Srgb,
            vk::ColorSpaceKHR::eSrgbNonlinear,
            extent,
            1,
            vk::ImageUsageFlagBits::eColorAttachment,
            vk::SharingMode::eExclusive,
            {},
            surfaceCapabilities.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            vk::PresentModeKHR::eFifo,
            false,
            oldSwapchain
        } },
        images { swapchain.getImages() },
        imageViews { [&]() {
            std::vector<vk::raii::ImageView> result;
            result.reserve(images.size());
            for (vk::Image image : images) {
                result.emplace_back(device, vk::ImageViewCreateInfo {
                    {},
                    image,
                    vk::ImageViewType::e2D,
                    vk::Format::eB8G8R8A8Srgb,
                    {},
                    vk::ImageSubresourceRange { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
                });
            }
            return result;
        }() },
        imageReadySemaphores { [&]() {
            std::vector<vk::raii::Semaphore> result;
            result.reserve(images.size());
            for (std::size_t i = 0; i < images.size(); ++i) {
                result.emplace_back(device, vk::SemaphoreCreateInfo{});
            }
            return result;
        }() } { }

private:
    [[nodiscard]] static std::uint32_t getMinImageCount(const vk::SurfaceCapabilitiesKHR &surfaceCapabilities) noexcept {
        std::uint32_t result = surfaceCapabilities.minImageCount + 1;
        if (surfaceCapabilities.maxImageCount != 0) {
            result = std::min(result, surfaceCapabilities.maxImageCount);
        }
        return result;
    }
};

class Frame {
public:
    class Shared {
        std::reference_wrapper<const Gpu> gpu;

    public:
        TriangleRenderPipeline triangleRenderPipeline;

        Swapchain swapchain;

        vk::raii::Semaphore timelineSemaphore;

        Shared(const Gpu &gpu, Swapchain &&_swapchain)
            : gpu { gpu }
            , triangleRenderPipeline { gpu.device }
            , swapchain { std::move(_swapchain) }
            , timelineSemaphore { gpu.device, vk::StructureChain {
                vk::SemaphoreCreateInfo{},
                vk::SemaphoreTypeCreateInfo { vk::SemaphoreType::eTimeline, 0 },
            }.get() } { }

        void setSwapchain(Swapchain &&_swapchain) {
            swapchain = std::move(_swapchain);
        }
    };

    Frame(const Gpu &gpu, std::shared_ptr<const Shared> _shared)
        : gpu { gpu }
        , shared { std::move(_shared) }
        , commandPool { gpu.device, vk::CommandPoolCreateInfo { {}, gpu.queueFamily } }
        , frameCommandBuffer { (*gpu.device).allocateCommandBuffers({ *commandPool, vk::CommandBufferLevel::ePrimary, 1 })[0] }
        , imageAvailableSemaphore { gpu.device, vk::SemaphoreCreateInfo{} } { }

    void waitForPreviousExecution(std::uint64_t frameIndex) const {
        std::ignore = gpu.get().device.waitSemaphores(vk::SemaphoreWaitInfo {
            {},
            *shared->timelineSemaphore,
            frameIndex,
        }, ~0ULL);
    }

    void execute(std::uint64_t frameIndex) {
        // Acquire swapchain image.
        std::uint32_t swapchainImageIndex;
        try {
            swapchainImageIndex = (*gpu.get().device).acquireNextImageKHR(*shared->swapchain.swapchain, ~0ULL, *imageAvailableSemaphore).value;
        }
        catch (const vk::OutOfDateKHRError&) {
            return;
        }

        // Record frame command buffer.
        commandPool.reset();
        frameCommandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        frameCommandBuffer.pipelineBarrier2({
            {}, {}, {},
            vku::lvalue(vk::ImageMemoryBarrier2 {
                vk::PipelineStageFlagBits2::eColorAttachmentOutput, {},
                vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite,
                {}, vk::ImageLayout::eColorAttachmentOptimal,
                vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                shared->swapchain.images[swapchainImageIndex], vku::fullSubresourceRange(vk::ImageAspectFlagBits::eColor),
            })
        });

        const vk::Rect2D renderArea { {}, shared->swapchain.extent };
        frameCommandBuffer.beginRendering(vk::RenderingInfo{
            {},
            renderArea,
            1,
            {},
            vku::lvalue(vk::RenderingAttachmentInfo {
                *shared->swapchain.imageViews[swapchainImageIndex], vk::ImageLayout::eColorAttachmentOptimal,
                {}, {}, {},
                vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ClearColorValue{},
            }),
        });

        frameCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *shared->triangleRenderPipeline.pipeline);

        frameCommandBuffer.setViewport(0, vku::toViewport(renderArea));
        frameCommandBuffer.setScissor(0, renderArea);

        frameCommandBuffer.draw(3, 1, 0, 0);
        
        frameCommandBuffer.endRendering();

        frameCommandBuffer.pipelineBarrier2({
            {}, {}, {},
            vku::lvalue(vk::ImageMemoryBarrier2 {
                vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite,
                vk::PipelineStageFlagBits2::eAllCommands, {},
                vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
                vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                shared->swapchain.images[swapchainImageIndex], vku::fullSubresourceRange(vk::ImageAspectFlagBits::eColor),
            })
        });

        frameCommandBuffer.end();

        // Submit frame command buffer.
        gpu.get().queue.submit2(vk::SubmitInfo2 {
            {},
            vku::lvalue({
                vk::SemaphoreSubmitInfo { *imageAvailableSemaphore, {}, vk::PipelineStageFlagBits2::eColorAttachmentOutput },
                vk::SemaphoreSubmitInfo { *shared->timelineSemaphore, frameIndex, vk::PipelineStageFlagBits2::eColorAttachmentOutput },
            }),
            vku::lvalue(vk::CommandBufferSubmitInfo { frameCommandBuffer }),
            vku::lvalue({
                vk::SemaphoreSubmitInfo { *shared->swapchain.imageReadySemaphores[swapchainImageIndex], {}, vk::PipelineStageFlagBits2::eAllCommands },
                vk::SemaphoreSubmitInfo { *shared->timelineSemaphore, frameIndex + 1, vk::PipelineStageFlagBits2::eColorAttachmentOutput },
            }),
        });

        // Present the acquired swapchain image.
        try {
            std::ignore = gpu.get().queue.presentKHR({
                *shared->swapchain.imageReadySemaphores[swapchainImageIndex],
                *shared->swapchain.swapchain,
                swapchainImageIndex,
            });
        }
        catch (const vk::OutOfDateKHRError&) { }
    }

private:
    std::reference_wrapper<const Gpu> gpu;
    std::shared_ptr<const Shared> shared;

    vk::raii::CommandPool commandPool;
    vk::CommandBuffer frameCommandBuffer;

    vk::raii::Semaphore imageAvailableSemaphore;
};

class App {
public:
    explicit App()
        : window { createWindow() }
        , instance { createInstance() }
        , surface { createSurface() }
        , gpu { std::move(instance.enumeratePhysicalDevices().at(0)), *surface }
        , frameShared { std::make_shared<Frame::Shared>(gpu, Swapchain {
            gpu.device,
            *surface,
            getFramebufferExtent(),
            gpu.physicalDevice.getSurfaceCapabilitiesKHR(surface),
        }) }
        , frames { Frame { gpu, frameShared }, Frame { gpu, frameShared } } {
        // Register the callback that recreates the swapchain and related resources when the window is resized.
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
            while (width == 0 || height == 0) {
                glfwWaitEvents();
                glfwGetFramebufferSize(window, &width, &height);
            }

            auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
            app->gpu.device.waitIdle();
            app->frameShared->setSwapchain({
                app->gpu.device,
                app->surface,
                vk::Extent2D { static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height) },
                app->gpu.physicalDevice.getSurfaceCapabilitiesKHR(app->surface),
                *app->frameShared->swapchain.swapchain,
            });
        });
    }

    ~App() {
        // Cleanup GLFW window.
        glfwDestroyWindow(window);
    }

    void run() {
        for (std::uint64_t frameIndex = 0; !glfwWindowShouldClose(window); ++frameIndex) {
            Frame &frame = frames[frameIndex % frames.size()];

            if (frameIndex >= frames.size()) {
                // Wait for the previous frame's execution.
                frame.waitForPreviousExecution(frameIndex);
            }

            // Handle window events.
            glfwPollEvents();

            // Execute the frame.
            frame.execute(frameIndex);
        }

        gpu.device.waitIdle();
    }

private:
    GLFWwindow *window;

    vk::raii::Context context;
    vk::raii::Instance instance;
    vk::raii::SurfaceKHR surface;

    Gpu gpu;

    std::shared_ptr<Frame::Shared> frameShared;
    std::array<Frame, 2> frames;

    [[nodiscard]] static GLFWwindow *createWindow() {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        return glfwCreateWindow(800, 480, "triangle_1_3", nullptr, nullptr);
    }

    [[nodiscard]] vk::raii::Instance createInstance() const {
    #if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
        // Initialize Vulkan function pointers.
        VULKAN_HPP_DEFAULT_DISPATCHER.init();
    #endif

        std::vector<const char*> extensions;

        for (const vk::ExtensionProperties &props : vk::enumerateInstanceExtensionProperties()) {
            if (static_cast<std::string_view>(props.extensionName) == vk::KHRPortabilityEnumerationExtensionName) {
                // This application supports the Vulkan portability subset.
                extensions.push_back(vk::KHRPortabilityEnumerationExtensionName);
                break;
            }
        }

        // Add required Vulkan instance extensions for GLFW.
        std::uint32_t glfwExtensionCount;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::copy_n(glfwExtensions, glfwExtensionCount, back_inserter(extensions));

        vk::raii::Instance result { context, vk::InstanceCreateInfo {
            vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
            &vku::lvalue(vk::ApplicationInfo {
                "triangle_1_3", 0,
                nullptr, 0,
                vk::makeApiVersion(0, 1, 3, 0),
            }),
            {},
            extensions,
        } };

    #if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
        // Initialize per-instance Vulkan function pointers.
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*result);
    #endif

        return result;
    }

    [[nodiscard]] vk::raii::SurfaceKHR createSurface() const {
        if (VkSurfaceKHR rawSurface; glfwCreateWindowSurface(*instance, window, nullptr, &rawSurface) == VK_SUCCESS) {
            return { instance, rawSurface };
        }

        throw std::runtime_error { "Failed to create Vulkan surface" };
    }

    [[nodiscard]] vk::Extent2D getFramebufferExtent() const {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        return { static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height) };
    }
};

int main() {
    glfwInit();
    try {
        App{}.run();
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    glfwTerminate();
}