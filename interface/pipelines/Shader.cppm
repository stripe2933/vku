module;

#include <cerrno>
#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <source_location>
#include <stdexcept>
#include <utility>
#include <vector>
#endif

export module vku:pipelines.Shader;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vulkan_hpp;

namespace vku {
    export struct Shader {
        std::vector<std::uint32_t> code;
        vk::ShaderStageFlagBits stage;
        const char *entryPoint;

        // --------------------
        // Constructors.
        // --------------------

        /**
         * Construct <tt>Shader</tt> from existing SPIR-V shader code vector.
         * @param code SPIR-V shader code. This will be moved to the struct.
         * @param stage Shader stage.
         * @param entryPoint Entry point function name (default: "main").
         */
        Shader(std::vector<std::uint32_t> code, vk::ShaderStageFlagBits stage, const char *entryPoint = "main");

        /**
         * Construct <tt>Shader</tt> from compiled SPIR-V file.
         * @param path Path to the compiled SPIR-V file.
         * @param stage Shader stage.
         * @param entryPoint Entry point function name (default: "main").
         */
        Shader(const std::filesystem::path &path, vk::ShaderStageFlagBits stage, const char *entryPoint = "main");
    };
}

// --------------------
// Implementations.
// --------------------

[[nodiscard]] auto loadFileAsBinary(
    const std::filesystem::path &path
) -> std::vector<std::uint32_t> {
    std::ifstream file { path, std::ios::binary };
    if (!file) {
        throw std::runtime_error { std::format("Failed to open file: {} (error code={})", std::strerror(errno), errno) };
    }

    file.seekg(0, std::ios::end);
    const auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<std::uint32_t> result(fileSize / sizeof(std::uint32_t));
    file.read(reinterpret_cast<char*>(result.data()), fileSize);

    return result;
}

vku::Shader::Shader(
    std::vector<std::uint32_t> code,
    vk::ShaderStageFlagBits stage,
    const char *entryPoint
) : code { std::move(code) },
    stage { stage },
    entryPoint { entryPoint } { }

vku::Shader::Shader(
    const std::filesystem::path &path,
    vk::ShaderStageFlagBits stage,
    const char *entryPoint
) : code { loadFileAsBinary(path) },
    stage { stage },
    entryPoint { entryPoint } { }