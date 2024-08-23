#include <cassert>
#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <algorithm>
#include <array>
#include <functional>
#include <ranges>
#include <span>
#include <tuple>
#include <vector>
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
                vk::EXTDescriptorIndexingExtensionName,
#if __APPLE__
                vk::KHRPortabilitySubsetExtensionName,
#endif
            },
            .devicePNexts = std::tuple {
                vk::PhysicalDeviceDescriptorIndexingFeatures{}
                    .setRuntimeDescriptorArray(true),
            },
            .apiVersion = vk::makeApiVersion(0, 1, 1, 0),
        } } { }
};

struct ColorCheckComputer {
    struct DescriptorSetLayout : vku::DescriptorSetLayout<vk::DescriptorType::eSampledImage, vk::DescriptorType::eStorageBuffer> {
        explicit DescriptorSetLayout(const vk::raii::Device &device [[clang::lifetimebound]], std::uint32_t mipLevels)
            : vku::DescriptorSetLayout<vk::DescriptorType::eSampledImage, vk::DescriptorType::eStorageBuffer> {
                device, vk::DescriptorSetLayoutCreateInfo {
                    {},
                    vku::unsafeProxy({
                        vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eSampledImage, mipLevels, vk::ShaderStageFlagBits::eCompute },
                        vk::DescriptorSetLayoutBinding { 1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute },
                    }),
                }
            } { }
    };

    struct PushConstant {
        std::array<float, 4> color;
        std::uint32_t mipLevel;
    };

    DescriptorSetLayout descriptorSetLayout;
    vk::raii::PipelineLayout pipelineLayout;
    vk::raii::Pipeline pipeline;

    explicit ColorCheckComputer(const vk::raii::Device &device [[clang::lifetimebound]], std::uint32_t mipLevels)
        : descriptorSetLayout { device, mipLevels }
        , pipelineLayout { device, vk::PipelineLayoutCreateInfo {
            {},
            *descriptorSetLayout,
            vku::unsafeProxy(vk::PushConstantRange {
                vk::ShaderStageFlagBits::eCompute,
                0, sizeof(PushConstant),
            }),
        } }
        , pipeline { device, nullptr, vk::ComputePipelineCreateInfo {
            {},
            createPipelineStages(device, vku::Shader { COMPILED_SHADER_DIR "/color_check.comp.spv", vk::ShaderStageFlagBits::eCompute }).get()[0],
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
            "vku_test_get_mip_view_create_infos", 0,
            {}, 0,
            vk::makeApiVersion(0, 1, 1, 0),
        }),
        {},
#if __APPLE__
        vku::unsafeProxy({
            vk::KHRPortabilityEnumerationExtensionName,
        }),
#endif
    } };
    const Gpu gpu { instance };

    constexpr std::array colors {
        vk::ClearColorValue { 0.f, 0.f, 0.f, 1.f }, // Black
        vk::ClearColorValue { 1.f, 0.f, 0.f, 1.f }, // Red
        vk::ClearColorValue { 0.f, 1.f, 0.f, 1.f }, // Green
        vk::ClearColorValue { 0.f, 0.f, 1.f, 1.f }, // Blue
        vk::ClearColorValue { 1.f, 1.f, 0.f, 1.f }, // Yellow
        vk::ClearColorValue { 0.f, 1.f, 1.f, 1.f }, // Cyan
        vk::ClearColorValue { 1.f, 0.f, 1.f, 1.f }, // Magenta
        vk::ClearColorValue { 1.f, 1.f, 1.f, 1.f }, // White
    };

    const vku::AllocatedImage image { gpu.allocator, vk::ImageCreateInfo {
        {},
        vk::ImageType::e2D,
        vk::Format::eR8G8B8A8Unorm,
        vk::Extent3D { 128, 128, 1 },
        colors.size(), 1,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
    } };
    const std::vector imageViews
        // --------------------
        // MAIN CODE TO TEST!
        // --------------------
        = image.getMipViewCreateInfos()
        | std::views::transform([&](const vk::ImageViewCreateInfo &createInfo) {
            return vk::raii::ImageView { gpu.device, createInfo };
        })
        | std::ranges::to<std::vector>();

    const vku::MappedBuffer buffer {
        gpu.allocator,
        std::from_range, std::vector(image.mipLevels, vk::True),
        vk::BufferUsageFlagBits::eStorageBuffer,
    };

    const ColorCheckComputer colorCheckComputer { gpu.device, image.mipLevels };

    const vk::raii::DescriptorPool descriptorPool { gpu.device, vk::DescriptorPoolCreateInfo {
        {},
        1,
        vku::unsafeProxy({
            vk::DescriptorPoolSize { vk::DescriptorType::eSampledImage, image.mipLevels },
            vk::DescriptorPoolSize { vk::DescriptorType::eStorageBuffer, 1 },
        }),
    } };
    const auto [descriptorSet] = vku::allocateDescriptorSets(*gpu.device, *descriptorPool, std::tie(colorCheckComputer.descriptorSetLayout));
    gpu.device.updateDescriptorSets({
        descriptorSet.getWrite<0>(vku::unsafeProxy(imageViews | std::views::transform([](const auto &imageView) {
            return vk::DescriptorImageInfo { {}, *imageView, vk::ImageLayout::eShaderReadOnlyOptimal };
        }) | std::ranges::to<std::vector>())),
        descriptorSet.getWriteOne<1>({ buffer, 0, vk::WholeSize }),
    }, {});

    const vk::raii::CommandPool computeCommandPool { gpu.device, vk::CommandPoolCreateInfo {
        {},
        gpu.queueFamilies.compute,
    } };

    vku::executeSingleCommand(*gpu.device, *computeCommandPool, gpu.queues.compute, [&](vk::CommandBuffer cb) {
        // Change image layout to TransferDstOptimal.
        cb.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer,
            {}, {}, {},
            vk::ImageMemoryBarrier {
                {}, vk::AccessFlagBits::eTransferWrite,
                {}, vk::ImageLayout::eTransferDstOptimal,
                vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                image, vku::fullSubresourceRange(),
            });

        // Fill image mipmaps with colors for each level, respectively.
        for (std::uint32_t level = 0; const vk::ClearColorValue &color : colors) {
            cb.clearColorImage(
                image, vk::ImageLayout::eTransferDstOptimal,
                color, vk::ImageSubresourceRange { vk::ImageAspectFlagBits::eColor, level, 1, 0, 1 });

            ++level;
        }

        // Change image layout to ShaderReadOnlyOptimal after clearing.
        cb.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader,
            {}, {}, {},
            vk::ImageMemoryBarrier {
                vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead,
                vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                image, vku::fullSubresourceRange(),
            });

        // Execute ColorCheckComputer for every mip levels.
        cb.bindPipeline(vk::PipelineBindPoint::eCompute, *colorCheckComputer.pipeline);
        cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *colorCheckComputer.pipelineLayout, 0, descriptorSet, {});
        for (std::uint32_t level = 0; const vk::ClearColorValue &color : colors) {
            cb.pushConstants<ColorCheckComputer::PushConstant>(*colorCheckComputer.pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, ColorCheckComputer::PushConstant {
                color.float32,
                level,
            });

            const vk::Extent2D mipExtent = image.mipExtent(level);
            constexpr auto divCeil = [](std::uint32_t num, std::uint32_t denom) noexcept {
                return num / denom + (num % denom != 0);
            };
            cb.dispatch(divCeil(mipExtent.width, 16U), divCeil(mipExtent.height, 16U), 1);

            ++level;
        }
    });
    gpu.queues.compute.waitIdle();

    // Check if all booleans in the buffer remain vk::True.
    assert(std::ranges::all_of(buffer.asRange<const vk::Bool32>(), std::identity{}) && "Color mismatch!");
}