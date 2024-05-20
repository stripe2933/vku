#pragma once

#include <vku/pipelines.hpp>

#ifdef NDEBUG
#include <resources/shaders.hpp>
#endif

class TriangleRenderer {
public:
    vk::raii::PipelineLayout pipelineLayout;
    vk::raii::Pipeline pipeline;

    explicit TriangleRenderer(
        const vk::raii::Device &device,
        vk::Format colorAttachmentFormat
    ) : pipelineLayout { createPipelineLayout(device) },
        pipeline { createPipeline(device, colorAttachmentFormat) } { }

    auto draw(
        vk::CommandBuffer commandBuffer
    ) const -> void {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
        commandBuffer.draw(3, 1, 0, 0);
    }

private:
    [[nodiscard]] static auto createPipelineLayout(
        const vk::raii::Device &device
    ) -> vk::raii::PipelineLayout {
        return { device, vk::PipelineLayoutCreateInfo{} };
    }

    [[nodiscard]] auto createPipeline(
        const vk::raii::Device &device,
        vk::Format colorAttachmentFormat
    ) const -> vk::raii::Pipeline {
        const auto [_, stages] = createStages(
            device,
            vku::Shader {
                vk::ShaderStageFlagBits::eVertex,
#ifdef NDEBUG

#else
                vku::Shader::readCode(VKU_EXAMPLES_01_HEADLESS_TRIANGLE_COMPILED_SHADER_DIR "/shaders/triangle.vert.spv"),
#endif
            },
            vku::Shader {
                vk::ShaderStageFlagBits::eFragment,
#ifdef NDEBUG

#else
                vku::Shader::readCode(VKU_EXAMPLES_01_HEADLESS_TRIANGLE_COMPILED_SHADER_DIR "/shaders/triangle.frag.spv"),
#endif
            });
        return { device, nullptr, vk::StructureChain {
            vku::getDefaultGraphicsPipelineCreateInfo(stages, *pipelineLayout, 1),
            // Use dynamic rendering.
            vk::PipelineRenderingCreateInfo {
                {},
                colorAttachmentFormat,
            },
        }.get() };
    }
};