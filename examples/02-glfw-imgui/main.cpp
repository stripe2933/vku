#include <set>
#include <thread>

#include <extlibs/ranges.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vku/AttachmentGroup.hpp>
#include <vku/Instance.hpp>
#include <vku/Swapchain.hpp>
#include <vku/GlfwWindow.hpp>
#include <vku/Gpu.hpp>
#include <vku/utils.hpp>

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
            drawImGuiCommandBuffer = (*app.device).allocateCommandBuffers(vk::CommandBufferAllocateInfo {
                *app.graphicsCommandPool,
                vk::CommandBufferLevel::ePrimary,
                1,
            })[0];
        }

        auto handleSwapchainResize() -> void {
            imGuiAttachmentGroups = createImGuiAttachmentGroups();
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
            drawImGui(imGuiAttachmentGroups[*imageIndex]);

            // Graphics queue submissions.
            constexpr vk::PipelineStageFlags drawImGuiWaitDstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            app.queues.graphics.submit(vk::SubmitInfo {
                *swapchainImageAcquireSema,
                drawImGuiWaitDstStage,
                drawImGuiCommandBuffer,
                *drawFinishSema,
            }, *inFlightFence);

            // Present swapchain image.
            if (!app.swapchain.presentImage(app.queues.present, *imageIndex, *drawFinishSema)) {
                return OnLoopResult::SwapchainOutdated;
            }

            return OnLoopResult::Success;
        }

    private:
        struct ImGuiAttachmentGroup final : vku::AttachmentGroup {
            ImGuiAttachmentGroup(
                const vk::raii::Device &device,
                vk::Image swapchainImage,
                const vk::Extent2D &extent
            ) : AttachmentGroup { extent } {
                addColorAttachment(
                    device,
                    vku::Image { swapchainImage, vk::Extent3D { extent, 1 }, vk::Format::eB8G8R8A8Srgb, 1, 1 },
                    vk::Format::eB8G8R8A8Unorm);
            }
        };

        const MainApp &app;

        // Attachment groups.
        std::vector<ImGuiAttachmentGroup> imGuiAttachmentGroups = createImGuiAttachmentGroups();

        // Command buffers.
        vk::CommandBuffer drawImGuiCommandBuffer;

        // Synchronization stuffs.
        vk::raii::Semaphore swapchainImageAcquireSema { app.device, vk::SemaphoreCreateInfo{} },
                            drawFinishSema { app.device, vk::SemaphoreCreateInfo{} };
        vk::raii::Fence     inFlightFence { app.device, vk::FenceCreateInfo { vk::FenceCreateFlagBits::eSignaled } } ;

        auto createImGuiAttachmentGroups() const -> std::vector<ImGuiAttachmentGroup> {
            return app.swapchain.getImages()
                | std::views::transform([this](vk::Image swapchainImage) {
                    return ImGuiAttachmentGroup { app.device, swapchainImage, app.swapchain.getExtent() };
                })
                | std::ranges::to<std::vector>();
        }

        auto update() const -> void {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::ShowDemoWindow();
            ImGui::Render();
        }

        auto drawImGui(
            const ImGuiAttachmentGroup &attachmentGroup
        ) const -> void {
            drawImGuiCommandBuffer.reset();
            drawImGuiCommandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

            // Layout transition for dynamic rendering.
            drawImGuiCommandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                {}, {}, {}, vk::ImageMemoryBarrier {
                    {}, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
                    {}, vk::ImageLayout::eColorAttachmentOptimal,
                    {}, {},
                    attachmentGroup.colorAttachments[0].image,
                    vku::fullSubresourceRange(),
                });

            // Begin dynamic rendering.
            drawImGuiCommandBuffer.beginRenderingKHR(attachmentGroup.getRenderingInfo(
                std::array {
                    std::tuple { vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ClearColorValue { 0.f, 0.f, 0.f, 1.f } },
                }
            ), *app.device.getDispatcher());

            // Draw ImGui.
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), drawImGuiCommandBuffer);

            // End dynamic rendering.
            drawImGuiCommandBuffer.endRenderingKHR(*app.device.getDispatcher());

            // Layout transition for present.
            drawImGuiCommandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe,
                {}, {}, {}, vk::ImageMemoryBarrier {
                    vk::AccessFlagBits::eColorAttachmentWrite, {},
                    vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
                    {}, {},
                    attachmentGroup.colorAttachments[0].image,
                    vku::fullSubresourceRange(),
                });

            drawImGuiCommandBuffer.end();
        }
    };

    MainApp()
        : Instance { createInstance() },
          GlfwWindow { 800, 480, "Vulkan + ImGui", instance },
          Gpu { createGpu() } {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_InitInfo initInfo {
            .Instance = *instance,
            .PhysicalDevice = *physicalDevice,
            .Device = *device,
            .Queue = queues.graphics,
            .DescriptorPool = *imGuiDescriptorPool,
            .MinImageCount = MAX_FRAMES_IN_FLIGHT,
            .ImageCount = MAX_FRAMES_IN_FLIGHT,
            .UseDynamicRendering = true,
            .ColorAttachmentFormat = VK_FORMAT_B8G8R8A8_UNORM,
        };
        ImGui_ImplVulkan_Init(&initInfo, nullptr);
    }
    ~MainApp() override {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    auto run() -> void {
        for (std::uint64_t frameIndex : std::views::iota(0ULL)) {
            glfwPollEvents();
            if (glfwWindowShouldClose(window)) {
                break;
            }

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

    vku::Swapchain<vk::ImageFormatListCreateInfo> swapchain = createSwapchain();
    vk::raii::DescriptorPool imGuiDescriptorPool = createImGuiDescriptorPool();
    vk::raii::CommandPool graphicsCommandPool = createCommandPool(queueFamilyIndices.graphics);
    std::array<Frame, MAX_FRAMES_IN_FLIGHT> frames = createFrames();

    [[nodiscard]] auto createGpu() const -> Gpu {
        return Gpu {
            instance,
            Gpu::Config<std::tuple<vk::PhysicalDeviceDynamicRenderingFeatures>> {
                .extensions = {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                    vk::KHRSwapchainExtensionName,
                    vk::KHRImageFormatListExtensionName, // Required by VK_KHR_swapchain_mutable_format.
                    vk::KHRSwapchainMutableFormatExtensionName,
                    vk::KHRMultiviewExtensionName, // Required by VK_KHR_create_renderpass2.
                    vk::KHRMaintenance2ExtensionName, // Required by VK_KHR_create_renderpass2.
                    vk::KHRCreateRenderpass2ExtensionName, // Required by VK_KHR_depth_stencil_resolve.
                    vk::KHRDepthStencilResolveExtensionName, // Required by VK_KHR_dynamic_rendering.
                    vk::KHRDynamicRenderingExtensionName,
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
                }
            },
        };
    }

    [[nodiscard]] auto createSwapchain() const -> decltype(swapchain) {
        const vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
        static constexpr std::array swapchainImageViewFormats {
            vk::Format::eB8G8R8A8Srgb,
            vk::Format::eB8G8R8A8Unorm, // For proper gamma correction, use B8G8R8A8_UNORM format image view for ImGui drawing.
        };
        return { device, vk::StructureChain {
            vk::SwapchainCreateInfoKHR {
                vk::SwapchainCreateFlagBitsKHR::eMutableFormat,
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
            },
            vk::ImageFormatListCreateInfo {
                swapchainImageViewFormats,
            },
        } };
    }

    [[nodiscard]] auto createImGuiDescriptorPool() const -> vk::raii::DescriptorPool {
        constexpr vk::DescriptorPoolSize poolSize { vk::DescriptorType::eCombinedImageSampler, 1 };
        return { device, vk::DescriptorPoolCreateInfo {
            vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            1,
            poolSize,
        } };
    }

    [[nodiscard]] auto createCommandPool(
        std::uint32_t queueFamilyIndex
    ) const -> vk::raii::CommandPool {
        return { device, vk::CommandPoolCreateInfo {
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            queueFamilyIndex,
        } };
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
                "GLFW + ImGui", 0,
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
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    MainApp{}.run();

    glfwTerminate();
    return 0;
}