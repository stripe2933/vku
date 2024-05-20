#include <extlibs/ranges.hpp>
#include <stb_image_write.h>
#include <vku/Allocator.hpp>
#include <vku/buffers.hpp>
#include <vku/commands.hpp>
#include <vku/Instance.hpp>
#include <vku/Gpu.hpp>
#include <vku/rendering.hpp>
#include <vulkan/vulkan_format_traits.hpp>

#include "pipelines/TriangleRenderer.hpp"

struct QueueFamilyIndices {
    std::uint32_t graphics;

    explicit QueueFamilyIndices(
        vk::PhysicalDevice physicalDevice
    ) {
        for (auto [queueFamilyIndex, properties] : physicalDevice.getQueueFamilyProperties() | ranges::views::enumerate) {
            if (properties.queueFlags & vk::QueueFlagBits::eCompute) {
                graphics = queueFamilyIndex;
                return;
            }
        }

        throw std::runtime_error { "Physical device does not support required queue families." };
    }
};

struct Queues {
    vk::Queue graphics;

    Queues(
        vk::Device device,
        const QueueFamilyIndices &queueFamilyIndices
    ) : graphics { device.getQueue(queueFamilyIndices.graphics, 0) } { }

    [[nodiscard]] static auto getDeviceQueueCreateInfos(
        const QueueFamilyIndices &queueFamilyIndices
    ) -> std::array<vk::DeviceQueueCreateInfo, 1> {
        static constexpr std::array queuePriorities { 1.f };
        return { vk::DeviceQueueCreateInfo { {}, queueFamilyIndices.graphics, queuePriorities } };
    }
};

class MainApp {
public:
    MainApp() = default;

    auto run() const -> void {
        // Create attachment group that has a color attachment for triangle rendering.
        struct AttachmentGroup : vku::AttachmentGroup {
            AttachmentGroup(
                const vk::raii::Device &device,
                vma::Allocator allocator
            ) : vku::AttachmentGroup { vk::Extent2D { 512, 512 } } {
                // Create vku::AllocateImage and move ownership into vku::AttachmentGroup.
                addColorAttachment(device, storeImage(std::make_unique<vku::AllocatedImage>(allocator, vk::ImageCreateInfo {
                    {},
                    vk::ImageType::e2D,
                    vk::Format::eB8G8R8A8Srgb,
                    vk::Extent3D { 512, 512, 1 },
                    1, 1,
                    vk::SampleCountFlagBits::e1,
                    vk::ImageTiling::eOptimal,
                    vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
                }, vma::AllocationCreateInfo {
                    {},
                    vma::MemoryUsage::eAutoPreferDevice,
                })));
            }
        } attachmentGroup { gpu.device, allocator };

        // Create pipeline.
        const TriangleRenderer triangleRenderer { gpu.device, attachmentGroup.colorAttachments[0].image.format };

        // Create host-visible buffer for tranfer destination.
        const vku::MappedBuffer destagingBuffer = vku::AllocatedBuffer { allocator, vk::BufferCreateInfo {
            {},
            static_cast<vk::DeviceSize>(blockSize(attachmentGroup.colorAttachments[0].image.format) * 512 * 512),
            vk::BufferUsageFlagBits::eTransferDst,
        }, vma::AllocationCreateInfo {
            vma::AllocationCreateFlagBits::eHostAccessRandom | vma::AllocationCreateFlagBits::eMapped,
            vma::MemoryUsage::eAuto,
        } };

        // Create command pool.
        const vk::raii::CommandPool graphicsCommandPool { gpu.device, vk::CommandPoolCreateInfo {
            {},
            gpu.queueFamilyIndices.graphics,
        } };

        // Render triangle
        vku::executeSingleCommand(*gpu.device, *graphicsCommandPool, gpu.queues.graphics, [&](vk::CommandBuffer commandBuffer) {
            // Change image layout to COLOR_ATTACHMENT_OPTIMAL.
            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                {}, {}, {}, vk::ImageMemoryBarrier {
                    {}, vk::AccessFlagBits::eColorAttachmentWrite,
                    {}, vk::ImageLayout::eColorAttachmentOptimal,
                    {}, {},
                    attachmentGroup.colorAttachments[0].image,
                    vku::fullSubresourceRange(),
                });

            // Begin dynamic rendering.
            commandBuffer.beginRenderingKHR(attachmentGroup.getRenderingInfo(std::array {
                std::tuple { vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ClearColorValue{} },
            }), *gpu.device.getDispatcher());

            // Viewport and scissor are set to dynamic state.
            attachmentGroup.setViewport(commandBuffer); // Set full viewport.
            attachmentGroup.setScissor(commandBuffer); // Set full scissor.

            // Dispatch pipeline.
            triangleRenderer.draw(commandBuffer);

            // End dynamic rendering.
            commandBuffer.endRenderingKHR(*gpu.device.getDispatcher());

            // Change image layout to TRANSFER_SRC_OPTIMAL.
            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eTransfer,
                {}, {}, {}, vk::ImageMemoryBarrier {
                    vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eTransferRead,
                    vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal,
                    {}, {},
                    attachmentGroup.colorAttachments[0].image,
                    vku::fullSubresourceRange(),
                });

            // Copy from color attachment to host-visible buffer.
            commandBuffer.copyImageToBuffer(
                attachmentGroup.colorAttachments[0].image, vk::ImageLayout::eTransferSrcOptimal,
                destagingBuffer,
                vk::BufferImageCopy {
                    0, 0, 0,
                    vk::ImageSubresourceLayers { vk::ImageAspectFlagBits::eColor, 0, 0, 1, },
                    {}, attachmentGroup.colorAttachments[0].image.extent,
                });
        });
        gpu.queues.graphics.waitIdle();

        // Write image to file.
        stbi_write_png("output.png", 512, 512, 4, destagingBuffer.data, 512 * 4);
    }

private:
    vku::Instance instance = createInstance();
    vku::Gpu<QueueFamilyIndices, Queues> gpu = createGpu();
    vku::Allocator allocator = createAllocator();

    [[nodiscard]] auto createGpu() const -> vku::Gpu<QueueFamilyIndices, Queues> {
        return vku::Gpu<QueueFamilyIndices, Queues> {
            instance.instance,
            vku::Gpu<QueueFamilyIndices, Queues>::Config<std::tuple<vk::PhysicalDeviceDynamicRenderingFeatures>> {
                // Internal physical device rater will check if the requested extension is available on the physical device, and ignore if not.
                .extensions = {
                    vk::KHRMultiviewExtensionName, // Required by VK_KHR_create_renderpass2.
                    vk::KHRMaintenance2ExtensionName, // Required by VK_KHR_create_renderpass2.
                    vk::KHRCreateRenderpass2ExtensionName, // Required by VK_KHR_depth_stencil_resolve.
                    vk::KHRDepthStencilResolveExtensionName, // Required by VK_KHR_dynamic_rendering.
                    vk::KHRDynamicRenderingExtensionName,
                },
                .pNexts = std::tuple {
                    vk::PhysicalDeviceDynamicRenderingFeatures { vk::True },
                }
            },
        };
    }

    [[nodiscard]] auto createAllocator() const -> vku::Allocator {
        return vku::Allocator { vma::AllocatorCreateInfo {
            {},
            *gpu.physicalDevice,
            *gpu.device,
            {}, {}, {}, {}, {},
            *instance.instance,
            vk::makeApiVersion(0, 1, 0, 0),
        } };
    }

    [[nodiscard]] static auto createInstance() -> vku::Instance {
        return vku::Instance {
            vk::ApplicationInfo {
                "Headless Triangle", 0,
                {}, 0,
                vk::makeApiVersion(0, 1, 0, 0),
            },
        };
    }
};

int main() {
    MainApp{}.run();
    return 0;
}