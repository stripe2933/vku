#pragma once

#include <vku/pipelines.hpp>

#ifdef NDEBUG
#include <resources/shaders.hpp>
#endif

class TriangleRenderer {
public:
    vk::raii::PipelineLayout pipelineLayout;
    vk::raii::Pipeline pipeline;

    TriangleRenderer(
        const vk::raii::Device &device,
        vk::Format colorAttachmentFormat
    ) : pipelineLayout { device, vk::PipelineLayoutCreateInfo{} },
        pipeline { createPipeline(device, colorAttachmentFormat) } { }

    auto draw(
        vk::CommandBuffer commandBuffer
    ) const -> void {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
        commandBuffer.draw(3, 1, 0, 0);
    }

private:
    [[nodiscard]] auto createPipeline(
        const vk::raii::Device &device,
        vk::Format colorAttachmentFormat
    ) const -> vk::raii::Pipeline {
        const auto [_, stages] = createStages(
            device,
            vku::Shader {
                vk::ShaderStageFlagBits::eVertex,
#ifdef NDEBUG
                vku::Shader::convert(resources::shaders_triangle_vert()),
#else
                vku::Shader::readCode(VKU_EXAMPLES_01_HEADLESS_TRIANGLE_COMPILED_SHADER_DIR "/shaders/triangle.vert.spv"),
#endif
            },
            vku::Shader {
                vk::ShaderStageFlagBits::eFragment,
#ifdef NDEBUG
                vku::Shader::convert(resources::shaders_triangle_frag()),
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