#pragma once

#include <filesystem>
#include <optional>

#include <vulkan/vulkan_raii.hpp>

namespace vku {
    struct Shader {
        vk::ShaderStageFlagBits stage;
        std::span<const std::uint32_t> code;
        const char *entryPoint = "main";
        std::optional<vk::SpecializationInfo> specializationInfo;

        [[nodiscard]] static auto readCode(const std::filesystem::path &path) -> std::vector<std::uint32_t>;

        template <typename T> requires (4 % sizeof(T) == 0)
        [[nodiscard]] static auto convert(
            std::span<const T> data
        ) noexcept -> std::span<const std::uint32_t> {
            return { reinterpret_cast<const std::uint32_t*>(data.data()), data.size() * sizeof(T) / sizeof(std::uint32_t) };
        }
    };
}