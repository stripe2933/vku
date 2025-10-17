#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stb_image.h>

#ifdef VKU_EXAMPLE_USE_MODULE
#include <vulkan/vulkan_hpp_macros.hpp>

import std;
import glm;
import vku;
#else
#include <cstdint>
#include <algorithm>
#include <array>
#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan_format_traits.hpp>
#include <vku.hpp>
#endif

#ifdef _WIN32
#define PATH_C_STR(...) (__VA_ARGS__).string().c_str()
#else
#define PATH_C_STR(...) (__VA_ARGS__).c_str()
#endif

std::filesystem::path texturePath;

class Gpu {
public:
    vk::raii::PhysicalDevice physicalDevice;
    std::uint32_t queueFamily;
    vk::raii::Device device;
    vk::Queue queue;
    vma::Allocator allocator;

    Gpu(const vk::raii::Instance &instance, vk::raii::PhysicalDevice &&_physicalDevice, vk::SurfaceKHR surface)
        : physicalDevice { std::move(_physicalDevice) }
        , queueFamily { getQueueFamily(surface) }
        , device { createDevice() }
        , queue { (*device).getQueue(queueFamily, 0) }
        , allocator { createAllocator(instance) } { }

    ~Gpu() {
        allocator.destroy();
    }

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
            vk::KHRMaintenance1ExtensionName,
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

        vk::raii::Device result { physicalDevice, vk::DeviceCreateInfo {
            {},
            vku::lvalue(vk::DeviceQueueCreateInfo {
                {},
                queueFamily,
                vk::ArrayProxyNoTemporaries<const float> { vku::lvalue(1.f) }
            }),
            {},
            extensions,
            &vk::PhysicalDeviceFeatures{}
                .setSamplerAnisotropy(true)
        } };

    #if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
        // Initialize per-device Vulkan function pointers.
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*result);
    #endif

        return result;
    }

    [[nodiscard]] vma::Allocator createAllocator(const vk::raii::Instance &instance) const {
        return vma::createAllocator(vma::AllocatorCreateInfo {
            {},
            *physicalDevice, *device,
            {}, {}, {}, {},
        #if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
            &vku::lvalue(vma::VulkanFunctions{
                instance.getDispatcher()->vkGetInstanceProcAddr,
                device.getDispatcher()->vkGetDeviceProcAddr,
            }),
        #else
            {},
        #endif
            *instance,
            vk::makeApiVersion(0, 1, 0, 0),
        });
    }
};

class CubeRenderPipeline {
public:
    struct PushConstant {
        static constexpr vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eVertex;

        glm::mat4 transform;
    };

    using DescriptorSetLayout = vku::raii::DescriptorSetLayout<vk::DescriptorType::eCombinedImageSampler>;

    vk::raii::Sampler sampler;
    DescriptorSetLayout descriptorSetLayout;
    vk::raii::PipelineLayout pipelineLayout;
    vk::raii::Pipeline pipeline;

    CubeRenderPipeline(const vk::raii::Device &device, vk::RenderPass renderPass)
        : sampler { device, vk::SamplerCreateInfo {
            {},
            vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
            vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
            0.f,
            false, 16.f,
            false, {},
            0.f, vk::LodClampNone,
        }.setMaxLod(vk::LodClampNone) }
        , descriptorSetLayout { device, vk::DescriptorSetLayoutCreateInfo {
            {},
            vku::lvalue(DescriptorSetLayout::getCreateInfoBinding<0>(vk::ShaderStageFlagBits::eFragment, *sampler)),
        } }
        , pipelineLayout { device, vk::PipelineLayoutCreateInfo {
            {},
            *descriptorSetLayout,
            vku::lvalue(vk::PushConstantRange {
                PushConstant::stages,
                0, sizeof(PushConstant),
            }),
        } }
        , pipeline { device, nullptr, vk::GraphicsPipelineCreateInfo {
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
            &vku::lvalue(vk::PipelineVertexInputStateCreateInfo {
                {},
                vku::lvalue(vk::VertexInputBindingDescription { 0, sizeof(float[5]), vk::VertexInputRate::eVertex }),
                vku::lvalue({
                    vk::VertexInputAttributeDescription { 0, 0, vk::Format::eR32G32B32Sfloat, 0 },
                    vk::VertexInputAttributeDescription { 1, 0, vk::Format::eR32G32Sfloat, sizeof(float[3]) },
                }),
            }),
            &vku::lvalue(vku::defaultPipelineInputAssemblyState(vk::PrimitiveTopology::eTriangleList)),
            nullptr,
            &vku::lvalue(vk::PipelineViewportStateCreateInfo {
                {},
                1, nullptr,
                1, nullptr,
            }),
            &vku::lvalue(vku::defaultPipelineRasterizationState(vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack)),
            &vku::lvalue(vk::PipelineMultisampleStateCreateInfo { {}, vk::SampleCountFlagBits::e1 }),
            &vku::lvalue(vk::PipelineDepthStencilStateCreateInfo {
                {},
                true, true, vk::CompareOp::eGreater /* Use reverse-Z */,
            }),
            &vku::lvalue(vku::defaultPipelineColorBlendState(1)),
            &vku::lvalue(vk::PipelineDynamicStateCreateInfo {
                {},
                vku::lvalue({ vk::DynamicState::eViewport, vk::DynamicState::eScissor }),
            }),
            *pipelineLayout,
            renderPass, 0,
        } } { }

private:
    static constexpr std::uint32_t vertexShaderCode[] = {
        #include "shaders/cube.vert.spv.h"
    };

    static constexpr std::uint32_t fragmentShaderCode[] = {
        #include "shaders/cube.frag.spv.h"
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
        vk::raii::RenderPass renderPass;

        CubeRenderPipeline cubeRenderPipeline;

        Swapchain swapchain;

        vku::raii::AllocatedBuffer cubeVertexBuffer;
        vku::raii::AllocatedImage cubeBaseColorImage;
        vk::raii::ImageView cubeBaseColorImageView;

        vk::raii::DescriptorPool descriptorPool;
        vku::DescriptorSet<CubeRenderPipeline::DescriptorSetLayout> cubeBaseColorTextureDescriptorSet;

        Shared(const Gpu &gpu, Swapchain &&_swapchain)
            : gpu { gpu }
            , renderPass { gpu.device, vk::RenderPassCreateInfo {
                {},
                vku::lvalue({
                    vk::AttachmentDescription {
                        {},
                        vk::Format::eB8G8R8A8Srgb,
                        vk::SampleCountFlagBits::e1,
                        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                        {}, {},
                        {}, vk::ImageLayout::ePresentSrcKHR,
                    },
                    vk::AttachmentDescription {
                        {},
                        vk::Format::eD32Sfloat,
                        vk::SampleCountFlagBits::e1,
                        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
                        {}, {},
                        {}, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                    },
                }),
                vku::lvalue(vk::SubpassDescription {
                    {},
                    vk::PipelineBindPoint::eGraphics,
                    {},
                    vku::lvalue(vk::AttachmentReference { 0, vk::ImageLayout::eColorAttachmentOptimal }),
                    {},
                    &vku::lvalue(vk::AttachmentReference { 1, vk::ImageLayout::eDepthStencilAttachmentOptimal }),
                }),
                vku::lvalue(vk::SubpassDependency {
                    vk::SubpassExternal, 0,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    {},
                    vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eColorAttachmentWrite,
                }),
            } }
            , cubeRenderPipeline { gpu.device, *renderPass }
            , swapchain { std::move(_swapchain) }
            , cubeVertexBuffer {
                gpu.allocator,
                vk::BufferCreateInfo {
                    {},
                    sizeof(float[5]) * 36,
                    vk::BufferUsageFlagBits::eVertexBuffer,
                },
                vma::AllocationCreateInfo {
                    vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
                    vma::MemoryUsage::eAutoPreferDevice,
                },
            }
            , cubeBaseColorImage { [&]() {
                int width, height;
                stbi_uc* const data = stbi_load(PATH_C_STR(texturePath), &width, &height, nullptr, STBI_rgb_alpha);

                vku::raii::AllocatedBuffer stagingBuffer {
                    gpu.allocator,
                    vk::BufferCreateInfo {
                        {},
                        blockSize(vk::Format::eR8G8B8A8Srgb) * static_cast<vk::DeviceSize>(width) * height,
                        vk::BufferUsageFlagBits::eTransferSrc,
                    },
                    vma::AllocationCreateInfo {
                        vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
                        vma::MemoryUsage::eAutoPreferHost,
                    },
                };
                gpu.allocator.copyMemoryToAllocation(data, stagingBuffer.allocation, 0, stagingBuffer.size);
                stbi_image_free(data);

                vku::raii::AllocatedImage result {
                    gpu.allocator,
                    vk::ImageCreateInfo {
                        {},
                        vk::ImageType::e2D,
                        vk::Format::eR8G8B8A8Srgb,
                        vk::Extent3D { static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), 1 },
                        1, 1,
                        vk::SampleCountFlagBits::e1,
                        vk::ImageTiling::eOptimal,
                        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                    },
                    vma::AllocationCreateInfo {
                        {},
                        vma::MemoryUsage::eAutoPreferDevice,
                    },
                };

                vk::raii::CommandPool commandPool { gpu.device, vk::CommandPoolCreateInfo { {}, gpu.queueFamily } };

                vku::executeSingleCommand(gpu.device, *commandPool, gpu.queue, [&](vk::CommandBuffer commandBuffer) {
                    commandBuffer.pipelineBarrier(
                        vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer,
                        {}, {}, {},
                        vk::ImageMemoryBarrier {
                            {}, vk::AccessFlagBits::eTransferWrite,
                            {}, vk::ImageLayout::eTransferDstOptimal,
                            vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                            result, vku::fullSubresourceRange(vk::ImageAspectFlagBits::eColor),
                        });

                    commandBuffer.copyBufferToImage(
                        stagingBuffer,
                        result, vk::ImageLayout::eTransferDstOptimal,
                        vk::BufferImageCopy {
                            0, 0, 0,
                            vk::ImageSubresourceLayers { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
                            vk::Offset3D{}, result.extent,
                        }
                    );

                    commandBuffer.pipelineBarrier(
                        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eBottomOfPipe,
                        {}, {}, {},
                        vk::ImageMemoryBarrier {
                            vk::AccessFlagBits::eTransferWrite, {},
                            vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                            vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                            result, vku::fullSubresourceRange(vk::ImageAspectFlagBits::eColor),
                        });
                });
                gpu.queue.waitIdle();

                return result;
            }() }
            , cubeBaseColorImageView { gpu.device, cubeBaseColorImage.getViewCreateInfo(vk::ImageViewType::e2D) }
            , descriptorPool { [&]() {
                vku::DescriptorPoolSize poolSize { cubeRenderPipeline.descriptorSetLayout };
                return vk::raii::DescriptorPool { gpu.device, vk::DescriptorPoolCreateInfo {
                    {},
                    poolSize.getMaxSets(),
                    vku::lvalue(poolSize.getPoolSizes()),
                } };
            }() } {
            constexpr float cubeVertices[] = {
                // Back face
                -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
                 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
                 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right
                 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
                -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
                -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
                // Front face
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
                 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
                 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
                 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
                -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
                // Left face
                -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
                -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
                -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
                // Right face
                 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
                 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
                 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
                 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
                 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
                 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
                // Bottom face
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
                 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
                 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
                 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
                // Top face
                -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
                 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
                 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
                 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
                -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
                -0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left
            };
            gpu.allocator.copyMemoryToAllocation(cubeVertices, cubeVertexBuffer.allocation, 0, cubeVertexBuffer.size);

            vku::DescriptorSetAllocationBuilder{}
                .add(cubeRenderPipeline.descriptorSetLayout, cubeBaseColorTextureDescriptorSet)
                .allocate(gpu.device, *descriptorPool);
            gpu.device.updateDescriptorSets(
                cubeBaseColorTextureDescriptorSet.getWrite<0>(0, vku::lvalue(vk::DescriptorImageInfo {
                    {},
                    *cubeBaseColorImageView,
                    vk::ImageLayout::eShaderReadOnlyOptimal,
                })),
                {});
        }

        void setSwapchain(Swapchain &&_swapchain) {
            swapchain = std::move(_swapchain);
        }
    };

    Frame(const Gpu &gpu, std::shared_ptr<const Shared> _shared)
        : gpu { gpu }
        , shared { std::move(_shared) }
        , depthImage { createDepthImage() }
        , depthImageView { gpu.device, depthImage.getViewCreateInfo(vk::ImageViewType::e2D) }
        , framebuffers { createFramebuffers() }
        , commandPool { gpu.device, vk::CommandPoolCreateInfo { {}, gpu.queueFamily } }
        , frameCommandBuffer { (*gpu.device).allocateCommandBuffers({ *commandPool, vk::CommandBufferLevel::ePrimary, 1 })[0] }
        , imageAvailableSemaphore { gpu.device, vk::SemaphoreCreateInfo{} }
        , frameReadyFence { gpu.device, vk::FenceCreateInfo{} } { }

    void waitForPreviousExecution() const {
        std::ignore = gpu.get().device.waitForFences(*frameReadyFence, true, ~0ULL);
    }

    void execute() {
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

        const vk::Rect2D renderArea { {}, shared->swapchain.extent };
        frameCommandBuffer.beginRenderPass({
            *shared->renderPass,
            *framebuffers[swapchainImageIndex],
            renderArea,
            vku::lvalue<vk::ClearValue>({
                vk::ClearColorValue { 0.f, 0.f, 0.f, 0.f },
                vk::ClearDepthStencilValue { 0.f, 0U },
            }),
        }, vk::SubpassContents::eInline);

        frameCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *shared->cubeRenderPipeline.pipeline);
        frameCommandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, *shared->cubeRenderPipeline.pipelineLayout,
            0, shared->cubeBaseColorTextureDescriptorSet, {});
        frameCommandBuffer.pushConstants<CubeRenderPipeline::PushConstant>(
            *shared->cubeRenderPipeline.pipelineLayout,
            CubeRenderPipeline::PushConstant::stages,
            0, CubeRenderPipeline::PushConstant {
                .transform = getProjectionViewMatrix() * transformMatrix,
            }
        );

        frameCommandBuffer.setViewport(0, vku::toViewport(renderArea, true));
        frameCommandBuffer.setScissor(0, renderArea);

        frameCommandBuffer.bindVertexBuffers(0, shared->cubeVertexBuffer.buffer, { 0 });

        frameCommandBuffer.draw(36, 1, 0, 0);
        
        frameCommandBuffer.endRenderPass();

        frameCommandBuffer.end();

        // Submit frame command buffer.
        gpu.get().device.resetFences(*frameReadyFence);
        gpu.get().queue.submit(vk::SubmitInfo {
            *imageAvailableSemaphore,
            vku::lvalue(vk::Flags { vk::PipelineStageFlagBits::eColorAttachmentOutput }),
            frameCommandBuffer,
            *shared->swapchain.imageReadySemaphores[swapchainImageIndex],
        }, *frameReadyFence);

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

    void setTransform(const glm::mat4 &transform) {
        transformMatrix = transform;
    }

    void handleSwapchainChange() {
        depthImage = createDepthImage();
        depthImageView = { gpu.get().device, depthImage.getViewCreateInfo(vk::ImageViewType::e2D) };
        framebuffers = createFramebuffers();
    }

private:
    glm::mat4 projectionViewMatrix;
    glm::mat4 transformMatrix;

    std::reference_wrapper<const Gpu> gpu;
    std::shared_ptr<const Shared> shared;

    vku::raii::AllocatedImage depthImage;
    vk::raii::ImageView depthImageView;

    std::vector<vk::raii::Framebuffer> framebuffers;

    vk::raii::CommandPool commandPool;
    vk::CommandBuffer frameCommandBuffer;

    vk::raii::Semaphore imageAvailableSemaphore;
    vk::raii::Fence frameReadyFence;

    [[nodiscard]] vku::raii::AllocatedImage createDepthImage() const {
        return vku::raii::AllocatedImage {
            gpu.get().allocator,
            vk::ImageCreateInfo {
                {},
                vk::ImageType::e2D,
                vk::Format::eD32Sfloat,
                vk::Extent3D { shared->swapchain.extent.width, shared->swapchain.extent.height, 1 },
                1, 1,
                vk::SampleCountFlagBits::e1,
                vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eDepthStencilAttachment,
            },
            vma::AllocationCreateInfo {
                {},
                vma::MemoryUsage::eAutoPreferDevice,
                {},
                vk::MemoryPropertyFlagBits::eLazilyAllocated,
            },
        };
    }

    [[nodiscard]] std::vector<vk::raii::Framebuffer> createFramebuffers() const {
        std::vector<vk::raii::Framebuffer> result;
        result.reserve(shared->swapchain.imageViews.size());
        for (const vk::raii::ImageView &imageView : shared->swapchain.imageViews) {
            result.emplace_back(gpu.get().device, vk::FramebufferCreateInfo {
                {},
                *shared->renderPass,
                vku::lvalue({ *imageView, *depthImageView }),
                shared->swapchain.extent.width, shared->swapchain.extent.height, 1,
            });
        }
        return result;
    }

    [[nodiscard]] glm::mat4 getProjectionViewMatrix() const {
        return glm::perspectiveRH_ZO(glm::radians(45.f), vku::aspect(shared->swapchain.extent), 20.f, 0.1f)
            * glm::lookAt(glm::vec3 { 3.f, 2.f, 5.f }, glm::vec3 { 0.f }, glm::vec3 { 0.f, 1.f, 0.f });
    }
};

class App {
public:
    explicit App()
        : window { createWindow() }
        , instance { createInstance() }
        , surface { createSurface() }
        , gpu { instance, std::move(instance.enumeratePhysicalDevices().at(0)), *surface }
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
            for (Frame &frame : app->frames) {
                frame.handleSwapchainChange();
            }
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
                frame.waitForPreviousExecution();
            }

            // Handle window events.
            glfwPollEvents();

            frame.setTransform(glm::rotate(glm::identity<glm::mat4>(), static_cast<float>(glfwGetTime()), glm::vec3 { 0.f, 1.f, 0.f }));

            // Execute the frame.
            frame.execute();
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
        return glfwCreateWindow(800, 480, "textured_cube", nullptr, nullptr);
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
                extensions.push_back(vk::KHRGetPhysicalDeviceProperties2ExtensionName);
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
                "textured_cube", 0,
                nullptr, 0,
                vk::makeApiVersion(0, 1, 0, 0),
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

int main(int argc, char **argv) {
    texturePath = std::filesystem::path { reinterpret_cast<const char8_t*>(argv[0]) }.parent_path() / "texture.jpg";

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