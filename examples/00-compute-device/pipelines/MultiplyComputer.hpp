#pragma once

#include <vku/DescriptorSetLayouts.hpp>
#include <vku/DescriptorSets.hpp>
#include <vku/pipelines.hpp>
#include <vku/RefHolder.hpp>
#include <vku/utils.hpp>

#ifdef NDEBUG
#include <resources/shaders.hpp>
#endif

class MultiplyComputer {
public:
    struct DescriptorSetLayouts : vku::DescriptorSetLayouts<1 /* binding count in set=0 */> {
        explicit DescriptorSetLayouts(
            const vk::raii::Device &device
        ) : vku::DescriptorSetLayouts<1> { device, LayoutBindings {
                {},
                vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute },
            } } { }
    };

    struct DescriptorSets : vku::DescriptorSets<DescriptorSetLayouts> {
        using vku::DescriptorSets<DescriptorSetLayouts>::DescriptorSets;

        [[nodiscard]] auto getDescriptorWrites0(
            const vk::DescriptorBufferInfo &bufferInfo
        ) const -> auto {
            // Use vku::RefHolder to make vk::WriteDescriptorSet "holds" the corresponding vk::DescriptorBufferInfo
            // reference.
            return vku::RefHolder {
                [this](const vk::DescriptorBufferInfo &bufferInfo) {
                    // Descriptor type are automatically set by predefined DescriptorSetLayouts parameters.
                    return getDescriptorWrite<0 /* Set index */, 0 /* Binding index */>()
                        .setBufferInfo(bufferInfo);
                },
                bufferInfo,
            };
        }
    };

    struct PushConstant {
        std::uint32_t numCount;
        float multiplier;
    };

    DescriptorSetLayouts descriptorSetLayouts;
    vk::raii::PipelineLayout pipelineLayout;
    vk::raii::Pipeline pipeline;

    explicit MultiplyComputer(
        const vk::raii::Device &device
    ) : descriptorSetLayouts { device },
        pipelineLayout { createPipelineLayout(device) },
        pipeline { createPipeline(device) } { }

    auto compute(
        vk::CommandBuffer commandBuffer,
        const DescriptorSets &descriptorSets,
        const PushConstant &pushConstant
    ) const -> void {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *pipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *pipelineLayout, 0, descriptorSets, {});
        commandBuffer.pushConstants<PushConstant>(*pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, pushConstant);
        commandBuffer.dispatch(vku::divCeil(pushConstant.numCount, 256U), 1, 1);
    }

private:
    [[nodiscard]] auto createPipelineLayout(
        const vk::raii::Device &device
    ) const -> vk::raii::PipelineLayout {
        constexpr vk::PushConstantRange pushConstantRange {
            vk::ShaderStageFlagBits::eCompute,
            0, sizeof(PushConstant)
        };
        return { device, vk::PipelineLayoutCreateInfo {
            {},
            descriptorSetLayouts,
            pushConstantRange,
        } };
    }

    [[nodiscard]] auto createPipeline(
        const vk::raii::Device &device
    ) const -> vk::raii::Pipeline {
        const auto [_ /* shader modules */, stages] = createStages(device, vku::Shader {
            vk::ShaderStageFlagBits::eCompute,
#ifdef NDEBUG
            vku::Shader::convert(resources::shaders_multiply_comp()),
#else
            vku::Shader::readCode(VKU_EXAMPLES_00_COMPUTE_DEVICE_COMPILED_SHADER_DIR "/shaders/multiply.comp.spv"),
#endif
        });
        return { device, nullptr, vk::ComputePipelineCreateInfo {
            {},
            get<0>(stages),
            pipelineLayout,
        } };
        // Shader modules must be alive until pipeline creation.
    }
};
