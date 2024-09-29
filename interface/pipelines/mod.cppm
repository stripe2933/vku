module;

#include <macros.hpp>

#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <array>
#include <bit>
#include <concepts>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:pipelines;
export import :pipelines.Shader;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
import :utils.RefHolder;

namespace vku {
    /**
     * Create array of <tt>vk::PipelineShaderStageCreateInfos</tt> from <tt>vku::Shader</tt>s.
     *
     * Creating compute pipeline:
     * @code{.cpp}
     * vk::raii::PipelineLayout pipelineLayout { device, vk::PipelineLayoutCreateInfo { ... } };
     * vk::raii::Pipeline pipeline { device, nullptr, vk::ComputePipelineCreateInfo {
     *     {},
     *     // vk::ComputePipelineCreateInfo accepts a single vk::PipelineShaderStageCreateInfo, therefore we call
     *     // vku::RefHolder<...>::get() to explicitly get the array, and access the first element.
     *     vku::createPipelineStages(
     *         device,
     *         vku::Shader { "shader.comp.spv", vk::ShaderStageFlagBits::eCompute }).get()[0],
     *     *pipelineLayout,
     * } }; // vk::raii::ShaderModules that are stored in the vku::createPipelineStages's return value will be destroyed here.
     * @endcode
     *
     * Creating graphics pipeline:
     * @code{.cpp}
     * vk::raii::Pipeline pipeline { device, nullptr, vk::GraphicsPipelineCreateInfo {
     *     {},
     *     vku::createPipelineStages(
     *         device,
     *         vku::Shader { "triangle.vert.spv", vk::ShaderStageFlagBits::eVertex },
     *         vku::Shader { "triangle.frag.spv", vk::ShaderStageFlagBits::eFragment }).get(),
     *     ... // Remaining pipeline settings.
     * } };
     * @endcode
     *
     * @param device Vulkan-Hpp RAII device that is used to create <tt>vk::raii::ShaderModule</tt>s.
     * @param shaders Variadic template parameters of <tt>vku::Shader</tt>s. This can be destroyed after the function
     * call (because <tt>vk::raii::ShaderModule</tt> created from the <tt>Shader</tt>s will be stored in the <tt>RefHolder</tt>).
     * @return vku::RefHolder of <tt>std::array<vk::PipelineShaderStageCreateInfo, sizeof...(shaders)></tt>, which can
     * be contextually converted to <tt>std::array<vk::PipelineShaderStageCreateInfo, sizeof...(shaders)></tt>. Each
     * create infos' corresponding <tt>vk::raii::ShaderModule</tt> will be stored in the return value.
     */
    export template <std::convertible_to<Shader>... Shaders>
    [[nodiscard]] auto createPipelineStages(const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device, const Shaders &...shaders)
#ifdef _MSC_VER
        -> RefHolder<std::array<VULKAN_HPP_NAMESPACE::PipelineShaderStageCreateInfo, sizeof...(Shaders)>, std::array<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ShaderModule, sizeof...(Shaders)>>
#endif
    {
        return RefHolder {
            [&](const auto &shaderModules) {
                return std::apply([&](const auto &...shaderModule) {
                    return std::array {
                        VULKAN_HPP_NAMESPACE::PipelineShaderStageCreateInfo {
                            {},
                            shaders.stage,
                            *shaderModule,
                            shaders.entryPoint,
                            shaders.specializationInfo.transform([](const auto &x) { return &x; }).value_or(nullptr),
                        }...
                    };
                }, shaderModules);
            },
            std::array {
                VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::ShaderModule { device, VULKAN_HPP_NAMESPACE::ShaderModuleCreateInfo {
                    {},
                    shaders.code,
                } }...
            },
        };
    }

    /**
     * A convenience function to create <tt>vk::GraphicsPipelineCreateInfo</tt> with mostly used pipeline parameters.
     *
     * Default pipeline parameters are:
     * - Vertex input state: no vertex buffer binding.
     * - Input assembly state: triangle list topology, no primitive restart.
     * - Viewport state: single viewport and scissor in dynamic state.
     * - Rasterization state: fill mode=fill, back face culling with counter-clockwise winding order, no depth clamp/bias,
     *   line width=1.
     * - Multisample state: rasterization samples=\p multisample, no sample shading and alpha to coverage/one.
     * - Depth-stencil state: <tt>nullptr</tt> if \p hasDepthStencilAttachment = <tt>false</tt>, disabled depth
     *   test/write otherwise.
     * - Color blend state: no blending, color write mask=all for all color attachments.
     * - Dynamic state: viewport and scissor.<br>
     * Each parameter in result struct are in static storage, therefore you don't have to care about their lifetime.
     *
     * Following code create a cube rendering graphics pipeline with vertex buffer of tightly-packed vec3, depth testing,
     * 4x MSAA using dynamic rendering (color format=B8G8R8A8Srgb, depth format=D32Sfloat):
     * @code{.cpp}
     * vk::raii::PipelineLayout pipelineLayout { device, vk::PipelineLayoutCreateInfo { ... } };
     * vk::raii::Pipeline pipeline { device, nullptr, vk::StructureChain {
     *     getDefaultGraphicsPipelineCreateInfo(
     *         vku::createPipelineStages(
     *             device,
     *             Shader { "cube.vert.spv", vk::ShaderStageFlagBits::eVertex },
     *             Shader { "cube.frag.spv", vk::ShaderStageFlagBits::eFragment }).get(),
     *         *pipelineLayout, 1, true, vk::SampleCountFlagBits::e4)
     *     .setPVertexInputState(vku::unsafeAddress(vk::PipelineVertexInputStateCreateInfo {
     *         {},
     *         vku::unsafeProxy({ vk::VertexInputBindingDescription { 0, 3 * sizeof(float), vk::VertexInputRate::eVertex } }),
     *         vku::unsafeProxy({ vk::VertexInputAttributeDescription { 0, 0, vk::Format::eR32G32B32Sfloat } }),
     *     })),
     *     .setPDepthStencilState(vku::unsafeAddress(vk::PipelineDepthStencilStateCreateInfo {
     *         {},
     *         true, true, vk::CompareOp::eLess,
     *     })),
     *     vk::PipelineRenderingCreateInfo {
     *         {},
     *         vku::unsafeProxy({ vk::Format::eB8G8R8A8Srgb }),
     *         vk::Format::eD32Sfloat,
     *     },
     * }.get() };
     * @endcode
     *
     * @param stages Pipeline shader stages that have to be used. You can use <tt>vku::createPipelineStages</tt> to make
     * it conveniently.
     * @param layout Pipeline layout that is used for the pipeline.
     * @param colorAttachmentCount Number of color attachments (default=<tt>0</tt>). This must be less than or equal to 8.
     * @param hasDepthStencilAttachment Boolean value that indicates whether the pipeline has depth-stencil attachment (default=<tt>false</tt>).
     * @param multisample MSAA sample count (default=<tt>vk::SampleCountFlagBits::e1</tt>).
     * @return <tt>vk::GraphicsPipelineCreateInfo</tt> struct. You may have to set <tt>renderPass</tt> and
     * <tt>subpass</tt> for conventional render-pass based pipeline creation, or set <tt>pNexts</tt> with
     * <tt>vk::PipelineRenderingCreateInfo[KHR]</tt> for dynamic rendering.
     * @throw std::runtime_error if \p colorAttachmentCount exceeds the maximum value (=8).
     */
    export
    [[nodiscard]] auto getDefaultGraphicsPipelineCreateInfo(
        VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const VULKAN_HPP_NAMESPACE::PipelineShaderStageCreateInfo> stages,
        VULKAN_HPP_NAMESPACE::PipelineLayout layout,
        std::uint32_t colorAttachmentCount = 0,
        bool hasDepthStencilAttachment = false,
        VULKAN_HPP_NAMESPACE::SampleCountFlagBits multisample = VULKAN_HPP_NAMESPACE::SampleCountFlagBits::e1
    ) -> VULKAN_HPP_NAMESPACE::GraphicsPipelineCreateInfo;
}

// --------------------
// Implementations.
// --------------------

#define INDEX_SEQ(Is, N, ...) [&]<std::size_t ...Is>(std::index_sequence<Is...>) __VA_ARGS__ (std::make_index_sequence<N>{})
#define ARRAY_OF(N, ...) INDEX_SEQ(Is, N, { return std::array { ((void)Is, __VA_ARGS__)... }; })

auto vku::getDefaultGraphicsPipelineCreateInfo(
    VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const VULKAN_HPP_NAMESPACE::PipelineShaderStageCreateInfo> stages,
    VULKAN_HPP_NAMESPACE::PipelineLayout layout,
    std::uint32_t colorAttachmentCount,
    bool hasDepthStencilAttachment,
    VULKAN_HPP_NAMESPACE::SampleCountFlagBits multisample
) -> VULKAN_HPP_NAMESPACE::GraphicsPipelineCreateInfo {
    static constexpr VULKAN_HPP_NAMESPACE::PipelineVertexInputStateCreateInfo vertexInputState{};

    static constexpr VULKAN_HPP_NAMESPACE::PipelineInputAssemblyStateCreateInfo inputAssemblyState {
        {},
        VULKAN_HPP_NAMESPACE::PrimitiveTopology::eTriangleList,
    };

    static constexpr VULKAN_HPP_NAMESPACE::PipelineViewportStateCreateInfo viewportState {
        {},
        1, {},
        1, {},
    };

    static constexpr VULKAN_HPP_NAMESPACE::PipelineRasterizationStateCreateInfo rasterizationState {
        {},
        {}, {},
        VULKAN_HPP_NAMESPACE::PolygonMode::eFill,
        VULKAN_HPP_NAMESPACE::CullModeFlagBits::eBack, {},
        {}, {}, {}, {},
        1.f,
    };

    static constexpr std::array multisampleStates {
        VULKAN_HPP_NAMESPACE::PipelineMultisampleStateCreateInfo { {}, VULKAN_HPP_NAMESPACE::SampleCountFlagBits::e1 },
        VULKAN_HPP_NAMESPACE::PipelineMultisampleStateCreateInfo { {}, VULKAN_HPP_NAMESPACE::SampleCountFlagBits::e2 },
        VULKAN_HPP_NAMESPACE::PipelineMultisampleStateCreateInfo { {}, VULKAN_HPP_NAMESPACE::SampleCountFlagBits::e4 },
        VULKAN_HPP_NAMESPACE::PipelineMultisampleStateCreateInfo { {}, VULKAN_HPP_NAMESPACE::SampleCountFlagBits::e8 },
        VULKAN_HPP_NAMESPACE::PipelineMultisampleStateCreateInfo { {}, VULKAN_HPP_NAMESPACE::SampleCountFlagBits::e16 },
        VULKAN_HPP_NAMESPACE::PipelineMultisampleStateCreateInfo { {}, VULKAN_HPP_NAMESPACE::SampleCountFlagBits::e32 },
        VULKAN_HPP_NAMESPACE::PipelineMultisampleStateCreateInfo { {}, VULKAN_HPP_NAMESPACE::SampleCountFlagBits::e64 },
    };

    static constexpr VULKAN_HPP_NAMESPACE::PipelineDepthStencilStateCreateInfo depthStencilState{};

    constexpr std::uint32_t MAX_COLOR_ATTACHMENT_COUNT = 8;
    if (colorAttachmentCount > MAX_COLOR_ATTACHMENT_COUNT) {
        throw std::runtime_error { "Color attachment count exceeds maximum" };
    }
    static constexpr std::array colorBlendAttachments
        = ARRAY_OF(MAX_COLOR_ATTACHMENT_COUNT + 1, VULKAN_HPP_NAMESPACE::PipelineColorBlendAttachmentState {
            {},
            {}, {}, {},
            {}, {}, {},
            VULKAN_HPP_NAMESPACE::ColorComponentFlagBits::eR | VULKAN_HPP_NAMESPACE::ColorComponentFlagBits::eG | VULKAN_HPP_NAMESPACE::ColorComponentFlagBits::eB | VULKAN_HPP_NAMESPACE::ColorComponentFlagBits::eA,
        });
    static constexpr std::array colorBlendStates
        = INDEX_SEQ(Is, MAX_COLOR_ATTACHMENT_COUNT + 1, {
            return std::array {
                VULKAN_HPP_NAMESPACE::PipelineColorBlendStateCreateInfo {
                    {},
                    {}, {},
                    Is, colorBlendAttachments.data(),
                }...
            };
        });

    static constexpr std::array dynamicStates {
        VULKAN_HPP_NAMESPACE::DynamicState::eViewport,
        VULKAN_HPP_NAMESPACE::DynamicState::eScissor,
    };
    static constexpr VULKAN_HPP_NAMESPACE::PipelineDynamicStateCreateInfo dynamicState {
        {},
        dynamicStates.size(), dynamicStates.data(),
    };

    return {
        {},
        stages,
        &vertexInputState,
        &inputAssemblyState,
        {},
        &viewportState,
        &rasterizationState,
        &multisampleStates[std::countr_zero(static_cast<std::underlying_type_t<VULKAN_HPP_NAMESPACE::SampleCountFlagBits>>(multisample))],
        hasDepthStencilAttachment ? &depthStencilState : nullptr,
        &colorBlendStates[colorAttachmentCount],
        &dynamicState,
        layout,
    };
}