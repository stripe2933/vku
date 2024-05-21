#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "details/reflect.hpp"
#include "Shader.hpp"

namespace vku {
namespace details {
    template <typename T>
    constexpr std::array<vk::SpecializationMapEntry, details::size<T>()> specializationMapEntries = []() {
        std::array<vk::SpecializationMapEntry, details::size<T>()> result;
        details::for_each([&](auto I) {
            get<I>(result)
                .setConstantID(I)
                .setOffset(details::offset_of<I, T>())
                .setSize(details::size_of<I, T>());
        }, result);
        return result;
    }();
}

    template <typename T>
    [[nodiscard]] auto getSpecializationInfo(
        const T &specializationData
    ) -> vk::SpecializationInfo {
        return { details::specializationMapEntries<T>, vk::ArrayProxyNoTemporaries(specializationData) };
    }

    template <std::convertible_to<Shader>... Shaders>
    [[nodiscard]] auto createStages(
        const vk::raii::Device &device,
        const Shaders &...shaders
    ) -> std::pair<std::array<vk::raii::ShaderModule, sizeof...(Shaders)>, std::array<vk::PipelineShaderStageCreateInfo, sizeof...(Shaders)>> {
        std::pair result {
            std::array { vk::raii::ShaderModule { device, vk::ShaderModuleCreateInfo {
                {},
                shaders.code,
            } }... },
            std::array { vk::PipelineShaderStageCreateInfo {
                {},
                shaders.stage,
                {},
                shaders.entryPoint,
                shaders.specializationInfo ? &*shaders.specializationInfo : nullptr,
            }... },
        };
        VKU_INDEX_SEQ(Is, sizeof...(Shaders), {
            (get<Is>(result.second).setModule(*get<Is>(result.first)), ...);
        });
        return result;
    }

    [[nodiscard]] auto getDefaultGraphicsPipelineCreateInfo(
        vk::ArrayProxyNoTemporaries<const vk::PipelineShaderStageCreateInfo> stages,
        vk::PipelineLayout layout,
        std::uint32_t colorAttachmentCount = 0,
        bool hasDepthStencilAttachemnt = false,
        vk::SampleCountFlagBits multisample = vk::SampleCountFlagBits::e1
    ) -> vk::GraphicsPipelineCreateInfo;
}
