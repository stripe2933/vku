#include <set>
#include <thread>

#include <extlibs/ranges.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vku/Allocator.hpp>
#include <vku/buffers.hpp>
#include <vku/commands.hpp>
#include <vku/Instance.hpp>
#include <vku/MsaaAttachmentGroup.hpp>
#include <vku/Swapchain.hpp>
#include <vku/GlfwWindow.hpp>
#include <vku/Gpu.hpp>
#include <vku/utils.hpp>

#include "pipelines/MeshRenderer.hpp"

#define INDEX_SEQ(Is, N, ...)                          \
    [&]<std::size_t ...Is>(std::index_sequence<Is...>) \
        __VA_ARGS__                                    \
    (std::make_index_sequence<N>{})
#define ARRAY_OF(N, ...)                            \
    INDEX_SEQ(Is, N, {                              \
        return std::array { (Is, __VA_ARGS__)... }; \
    })

struct QueueFamilyIndices {
    std::uint32_t graphics;
    std::uint32_t present;

    QueueFamilyIndices(
        vk::PhysicalDevice physicalDevice,
        vk::SurfaceKHR surface
    ) {
        for (std::optional<std::uint32_t> graphicsIndex, presentIndex;
             auto [queueFamilyIndex, properties] : physicalDevice.getQueueFamilyProperties() | ranges::views::enumerate) {
            if (properties.queueFlags & vk::QueueFlagBits::eGraphics) {
                graphicsIndex = queueFamilyIndex;
            }
            if (physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, surface)) {
                presentIndex = queueFamilyIndex;
            }

            if (graphicsIndex && presentIndex) {
                graphics = *graphicsIndex;
                present = *presentIndex;
                return;
            }
        }

        throw std::runtime_error { "Physical device does not support required queue families." };
    }
};

struct Queues {
    vk::Queue graphics;
    vk::Queue present;

    Queues(
        vk::Device device,
        const QueueFamilyIndices &queueFamilyIndices
    ) : graphics { device.getQueue(queueFamilyIndices.graphics, 0) },
        present { device.getQueue(queueFamilyIndices.present, 0) } { }

    [[nodiscard]] static auto getDeviceQueueCreateInfos(
        const QueueFamilyIndices &queueFamilyIndices
    ) -> std::vector<vk::DeviceQueueCreateInfo> {
        return std::set { queueFamilyIndices.graphics, queueFamilyIndices.present }
            | std::views::transform([](std::uint32_t queueFamilyIndex) {
                static constexpr std::array queuePriorities { 1.f };
                return vk::DeviceQueueCreateInfo { {}, queueFamilyIndex, queuePriorities };
            })
            | std::ranges::to<std::vector>();
    }
};

class MainApp final : vku::Instance, vku::GlfwWindow, vku::Gpu<QueueFamilyIndices, Queues> {
public:
    class Frame {
    public:
        enum class OnLoopResult {
            Success,
            SwapchainOutdated,
        };

        explicit Frame(
            const MainApp &app
        ) : app { app } {
            drawCommandBuffer = (*app.device).allocateCommandBuffers(vk::CommandBufferAllocateInfo {
                *app.graphicsCommandPool,
                vk::CommandBufferLevel::ePrimary,
                1,
            })[0];
        }

        auto handleSwapchainResize() -> void {
            swapchainAttachmentGroups = createSwapchainAttachmentGroups();
        }

        [[nodiscard]] auto onLoop() -> OnLoopResult {
            // Wait for the previous frame to finish.
            if (app.device.waitForFences(*inFlightFence, true, std::numeric_limits<std::uint64_t>::max()) != vk::Result::eSuccess) {
                throw std::runtime_error { "Failed to wait for in-flight fences" };
            }

            // Update.
            update();

            const std::optional imageIndex = app.swapchain.acquireImage(*swapchainImageAcquireSema);
            if (!imageIndex) return OnLoopResult::SwapchainOutdated;

            app.device.resetFences(*inFlightFence);

            // Draw ImGui.
            draw(swapchainAttachmentGroups[*imageIndex]);

            // Graphics queue submissions.
            constexpr vk::PipelineStageFlags drawImGuiWaitDstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            app.queues.graphics.submit(vk::SubmitInfo {
                *swapchainImageAcquireSema,
                drawImGuiWaitDstStage,
                drawCommandBuffer,
                *drawFinishSema,
            }, *inFlightFence);

            // Present swapchain image.
            if (!app.swapchain.presentImage(app.queues.present, *imageIndex, *drawFinishSema)) {
                return OnLoopResult::SwapchainOutdated;
            }

            return OnLoopResult::Success;
        }

    private:
        struct SwapchainAttachmentGroup final : vku::MsaaAttachmentGroup {
            SwapchainAttachmentGroup(
                const vk::raii::Device &device,
                vma::Allocator allocator,
                vk::Image swapchainImage,
                const vk::Extent2D &extent
            ) : MsaaAttachmentGroup { extent, vk::SampleCountFlagBits::e4 } {
                addColorAttachment(
                    device,
                    storeImage(createColorImage(allocator, vk::Format::eB8G8R8A8Srgb)),
                    vku::Image { swapchainImage, vk::Extent3D { extent, 1 }, vk::Format::eB8G8R8A8Srgb, 1, 1 });
                setDepthAttachment(
                    device,
                    storeImage(createDepthStencilImage(allocator, vk::Format::eD32Sfloat)));
            }
        };

        const MainApp &app;

        // Attachment groups.
        std::vector<SwapchainAttachmentGroup> swapchainAttachmentGroups = createSwapchainAttachmentGroups();

        // Command buffers.
        vk::CommandBuffer drawCommandBuffer;

        // Synchronization stuffs.
        vk::raii::Semaphore swapchainImageAcquireSema { app.device, vk::SemaphoreCreateInfo{} },
                            drawFinishSema { app.device, vk::SemaphoreCreateInfo{} };
        vk::raii::Fence     inFlightFence { app.device, vk::FenceCreateInfo { vk::FenceCreateFlagBits::eSignaled } } ;

        auto createSwapchainAttachmentGroups() const -> std::vector<SwapchainAttachmentGroup> {
            return { std::from_range, app.swapchain.getImages() | std::views::transform([this](vk::Image swapchainImage) {
                return SwapchainAttachmentGroup { app.device, app.allocator, swapchainImage, app.swapchain.getExtent() };
            }) };
        }

        auto update() -> void { }

        auto draw(
            const SwapchainAttachmentGroup &attachmentGroup
        ) const -> void {
            drawCommandBuffer.reset();
            drawCommandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

            // Layout transition for dynamic rendering.
            {
                const std::array barriers {
                    vk::ImageMemoryBarrier2 {
                        {}, {},
                        vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite,
                        {}, vk::ImageLayout::eColorAttachmentOptimal,
                        {}, {},
                        attachmentGroup.colorAttachments[0].image,
                        vku::fullSubresourceRange(),
                    },
                    vk::ImageMemoryBarrier2 {
                        {}, {},
                        vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite,
                        {}, vk::ImageLayout::eColorAttachmentOptimal,
                        {}, {},
                        attachmentGroup.colorAttachments[0].resolveImage,
                        vku::fullSubresourceRange(),
                    },
                    vk::ImageMemoryBarrier2 {
                        {}, {},
                        vk::PipelineStageFlagBits2::eEarlyFragmentTests, vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                        {}, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                        {}, {},
                        attachmentGroup.depthStencilAttachment->image,
                        vku::fullSubresourceRange(vk::ImageAspectFlagBits::eDepth),
                    },
                };
                drawCommandBuffer.pipelineBarrier2KHR({
                    {},
                    {}, {}, barriers,
                });
            }

            // Begin dynamic rendering.
            drawCommandBuffer.beginRenderingKHR(attachmentGroup.getRenderingInfo(
                std::array {
                    std::tuple { vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ClearColorValue { 0.f, 0.f, 0.f, 1.f } },
                },
                std::tuple { vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, vk::ClearDepthStencilValue { 1.f, 0U } }
            ));

            // Set viewport and scissor.
            attachmentGroup.setViewport(drawCommandBuffer, true); // Use negative viewport for OpenGL-compatible vertex data.
            attachmentGroup.setScissor(drawCommandBuffer);

            // Draw cube.
            app.meshRenderer.bindPipeline(drawCommandBuffer);
            drawCommandBuffer.bindVertexBuffers(0, app.cubeVertexBuffer.buffer, { 0 });
            app.meshRenderer.pushConstant(drawCommandBuffer, {
                glm::perspective(glm::radians(45.f), vku::aspect(app.swapchain.getExtent()), 0.5f, 10.f)
                    * lookAt(glm::vec3 { 3.f }, glm::vec3 { 0.f }, glm::vec3 { 0.f, 1.f, 0.f })
                    * app.model,
            });
            drawCommandBuffer.draw(36, 1, 0, 0);

            // End dynamic rendering.
            drawCommandBuffer.endRenderingKHR();

            // Layout transition for present.
            drawCommandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe,
                {}, {}, {},
                vk::ImageMemoryBarrier {
                    vk::AccessFlagBits::eColorAttachmentWrite, {},
                    vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
                    {}, {},
                    attachmentGroup.colorAttachments[0].resolveImage,
                    vku::fullSubresourceRange(),
                });

            drawCommandBuffer.end();
        }
    };

    glm::mat4 model { 1.f };

    MainApp()
        : Instance { createInstance() },
          GlfwWindow { 800, 480, "Cube in flight", instance },
          Gpu { createGpu() } { }

    auto run() -> void {
        for (float elapsed = 0.f; std::uint64_t frameIndex : std::views::iota(0ULL)) {
            glfwPollEvents();
            if (glfwWindowShouldClose(window)) {
                break;
            }

            const float timeDelta = glfwGetTime() - elapsed;
            elapsed += timeDelta;

            // Rotate cube model.
            model = rotate(model, timeDelta, glm::vec3 { 0.f, -1.f, 0.f });

            // Execute frame commands.
            switch (frames[frameIndex % MAX_FRAMES_IN_FLIGHT].onLoop()) {
            case Frame::OnLoopResult::Success:
                break;
            case Frame::OnLoopResult::SwapchainOutdated:
                device.waitIdle();

                // Yield while window is minimized.
                glm::ivec2 framebufferSize;
                while (!glfwWindowShouldClose(window) && (framebufferSize = getFramebufferSize()) == glm::ivec2 { 0 }) {
                    std::this_thread::yield();
                }

                // Recreate swapchain and related stuffs.
                swapchain.changeExtent(vku::convertExtent2D(framebufferSize));
                handleSwapchainResize();
                break;
            }
        }
        device.waitIdle();
    }

private:
    static constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    vku::Swapchain<> swapchain = createSwapchain();
    vku::Allocator allocator = createAllocator();
    MeshRenderer meshRenderer = createMeshRenderer();
    vk::raii::CommandPool graphicsCommandPool = createCommandPool(queueFamilyIndices.graphics);
    vku::AllocatedBuffer cubeVertexBuffer = createCubeVertexBuffer();
    std::array<Frame, MAX_FRAMES_IN_FLIGHT> frames = createFrames();

    [[nodiscard]] auto createGpu() const -> Gpu {
        return Gpu {
            instance,
            Gpu::Config<std::tuple<vk::PhysicalDeviceDynamicRenderingFeatures, vk::PhysicalDeviceSynchronization2Features>> {
                .extensions = {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                    vk::KHRMaintenance1ExtensionName,
                    vk::KHRSwapchainExtensionName,
                    vk::KHRImageFormatListExtensionName, // Required by VK_KHR_swapchain_mutable_format.
                    vk::KHRSwapchainMutableFormatExtensionName,
                    vk::KHRMultiviewExtensionName, // Required by VK_KHR_create_renderpass2.
                    vk::KHRMaintenance2ExtensionName, // Required by VK_KHR_create_renderpass2.
                    vk::KHRCreateRenderpass2ExtensionName, // Required by VK_KHR_depth_stencil_resolve.
                    vk::KHRDepthStencilResolveExtensionName, // Required by VK_KHR_dynamic_rendering.
                    vk::KHRDynamicRenderingExtensionName,
                    vk::KHRSynchronization2ExtensionName,
#pragma clang diagnostic pop
                },
                // For default, vku::Gpu tests that the physical device has required queue families by passed argument.
                // Our QueueFamilyIndices constructor has two arguments (vk::PhysicalDevice, vk::SurfaceKHR), therefore
                // queueFamilyIndicesGetter have to be manually specified.
                .queueFamilyIndicesGetter = [this](vk::PhysicalDevice physicalDevice) {
                    return QueueFamilyIndices { physicalDevice, *surface };
                },
                .pNexts = std::tuple {
                    vk::PhysicalDeviceDynamicRenderingFeatures { vk::True },
                    vk::PhysicalDeviceSynchronization2Features { vk::True },
                }
            },
        };
    }

    [[nodiscard]] auto createAllocator() const -> vku::Allocator {
        return vku::Allocator { vma::AllocatorCreateInfo {
            {},
            *physicalDevice,
            *device,
            {}, {}, {}, {}, {},
            *instance,
            vk::makeApiVersion(0, 1, 0, 0),
        } };
    }

    [[nodiscard]] auto createSwapchain() const -> vku::Swapchain<> {
        const vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
        return { device, vk::SwapchainCreateInfoKHR {
            {},
            *surface,
            std::min(capabilities.minImageCount + 1, capabilities.maxImageCount),
            vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear,
            vku::convertExtent2D(getFramebufferSize()),
            1,
            vk::ImageUsageFlagBits::eColorAttachment,
            {}, {},
            capabilities.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            vk::PresentModeKHR::eFifo,
        } };
    }

    [[nodiscard]] auto createMeshRenderer() const -> MeshRenderer {
        constexpr vk::VertexInputBindingDescription vertexBindingDescription {
            0,
            sizeof(glm::vec3) + sizeof(glm::vec2),
            vk::VertexInputRate::eVertex,
        };
        constexpr std::array vertexAttributeDescriptions {
            // inPosition
            vk::VertexInputAttributeDescription {
                0,
                0,
                vk::Format::eR32G32B32Sfloat,
                0,
            },
            // inTexcoord
            vk::VertexInputAttributeDescription {
                1,
                0,
                vk::Format::eR32G32Sfloat,
                sizeof(glm::vec3),
            },
        };
        return { device, vk::PipelineVertexInputStateCreateInfo {
            {},
            vertexBindingDescription,
            vertexAttributeDescriptions,
        }, vk::Format::eB8G8R8A8Srgb, vk::Format::eD32Sfloat };
    }

    [[nodiscard]] auto createCommandPool(
        std::uint32_t queueFamilyIndex
    ) const -> vk::raii::CommandPool {
        return { device, vk::CommandPoolCreateInfo {
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            queueFamilyIndex,
        } };
    }

    [[nodiscard]] auto createCubeVertexBuffer() const -> vku::AllocatedBuffer {
        // Data from LearnOpenGL: https://learnopengl.com/code_viewer.php?code=advanced/faceculling_vertexdata
        const vku::MappedBuffer stagingBuffer {
            allocator,
            std::from_range, std::vector {
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
            },
            vk::BufferUsageFlagBits::eTransferSrc,
        };
        vku::AllocatedBuffer vertexBuffer { allocator, vk::BufferCreateInfo {
            {},
            stagingBuffer.size,
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        }, vma::AllocationCreateInfo {
            {},
            vma::MemoryUsage::eAutoPreferDevice,
        } };

        vku::executeSingleCommand(*device, *graphicsCommandPool, queues.graphics, [&](vk::CommandBuffer commandBuffer) {
            commandBuffer.copyBuffer(
                stagingBuffer, vertexBuffer,
                vk::BufferCopy { 0, 0, vertexBuffer.size });
        });
        queues.graphics.waitIdle();

        return vertexBuffer;
    }

    [[nodiscard]] auto createFrames() const -> std::array<Frame, MAX_FRAMES_IN_FLIGHT> {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
        return ARRAY_OF(MAX_FRAMES_IN_FLIGHT, Frame { *this });
#pragma clang diagnostic pop
    }

    auto handleSwapchainResize() -> void {
        for (Frame &frame: frames) {
            frame.handleSwapchainResize();
        }
    }

    [[nodiscard]] static auto createInstance() -> Instance {
        return Instance {
            vk::ApplicationInfo {
                "Cube in flight", 0,
                {}, 0,
                vk::makeApiVersion(0, 1, 0, 0),
            },
            Instance::Config {
                .extensions = getInstanceExtensions(),
            }
        };
    }
};

int main() {
    VULKAN_HPP_DEFAULT_DISPATCHER.init();

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    MainApp{}.run();

    glfwTerminate();
    return 0;
}