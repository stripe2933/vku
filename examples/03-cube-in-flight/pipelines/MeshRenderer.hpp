#pragma once

#include <glm/glm.hpp>
#include <vku/pipelines.hpp>

#ifdef NDEBUG
#include <resources/shaders.hpp>
#endif

class MeshRenderer {
public:
    struct PushConstant {
        glm::mat4 transform;
    };

    vk::raii::PipelineLayout pipelineLayout;
    vk::raii::Pipeline pipeline;

    MeshRenderer(
        const vk::raii::Device &device,
        const vk::PipelineVertexInputStateCreateInfo &vertexInputState,
        vk::Format colorAttachmentFormat,
        vk::Format depthAttachmentFormat
    ) : pipelineLayout { createPipelineLayout(device) },
        pipeline { createPipeline(device, vertexInputState, colorAttachmentFormat, depthAttachmentFormat) } { }

    auto bindPipeline(
        vk::CommandBuffer commandBuffer
    ) const -> void {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
    }

    auto pushConstant(
        vk::CommandBuffer commandBuffer,
        const PushConstant &pushConstant
    ) const -> void {
        commandBuffer.pushConstants<PushConstant>(*pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, pushConstant);
    }

private:
    [[nodiscard]] auto createPipeline(
        const vk::raii::Device &device,
        const vk::PipelineVertexInputStateCreateInfo &vertexInputState,
        vk::Format colorAttachmentFormat,
        vk::Format depthAttachmentFormat
    ) const -> vk::raii::Pipeline {
        const auto [_, stages] = createStages(
            device,
            vku::Shader {
                vk::ShaderStageFlagBits::eVertex,
#ifdef NDEBUG
                vku::Shader::convert(resources::shaders_mesh_vert()),
#else
                vku::Shader::readCode(VKU_EXAMPLES_03_CUBE_IN_FLIGHT_COMPILED_SHADER_DIR "/shaders/mesh.vert.spv"),
#endif
            },
            vku::Shader {
                vk::ShaderStageFlagBits::eFragment,
#ifdef NDEBUG
                vku::Shader::convert(resources::shaders_mesh_frag()),
#else
                vku::Shader::readCode(VKU_EXAMPLES_03_CUBE_IN_FLIGHT_COMPILED_SHADER_DIR "/shaders/mesh.frag.spv"),
#endif
            });

        return { device, nullptr, vk::StructureChain {
            vku::getDefaultGraphicsPipelineCreateInfo(stages, *pipelineLayout, 1, true, vk::SampleCountFlagBits::e4)
                .setPVertexInputState(&vertexInputState),
            // Use dynamic rendering.
            vk::PipelineRenderingCreateInfo {
                {},
                colorAttachmentFormat,
                depthAttachmentFormat,
            },
        }.get() };
    }

    [[nodiscard]] static auto createPipelineLayout(
        const vk::raii::Device &device
    ) -> vk::raii::PipelineLayout {
        constexpr vk::PushConstantRange pushConstantRange {
            vk::ShaderStageFlagBits::eVertex,
            0, sizeof(PushConstant)
        };
        return { device, vk::PipelineLayoutCreateInfo {
            {},
            {},
            pushConstantRange,
        } };
    }
};