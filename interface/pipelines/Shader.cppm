/** @file pipelines/Shader.cppm
 */

module;

#include <cassert>
#include <cerrno>
#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
#endif

#ifdef VKU_USE_SHADERC
#include <ranges>
#include <source_location>
#include <string_view>

#include <shaderc/shaderc.hpp>
#endif
#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:pipelines.Shader;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vulkan_hpp;
import :utils.RefHolder;
#ifdef VKU_USE_SHADERC
import :details.to_string;
#endif

#ifdef _MSC_VER
#define PATH_C_STR(...) (__VA_ARGS__).string().c_str()
#else
#define PATH_C_STR(...) (__VA_ARGS__).c_str()
#endif

[[nodiscard]] auto loadFileAsBinary(
    const std::filesystem::path &path
) -> std::vector<std::byte> {
    std::ifstream file { path, std::ios::binary };
    if (!file) {
        throw std::runtime_error { std::format("Failed to open file: {} (error code={})", std::strerror(errno), errno) };
    }

    file.seekg(0, std::ios::end);
    const auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<std::byte> result(fileSize);
    file.read(reinterpret_cast<char*>(result.data()), fileSize);

    return result;
}

template <typename T, typename U>
[[nodiscard]] constexpr auto as(std::span<U> span) noexcept -> std::span<T> {
    assert(span.size_bytes() % sizeof(T) == 0 && "Size of span must be a multiple of the size of T");
    return { reinterpret_cast<T*>(span.data()), span.size_bytes() / sizeof(T) };
}

#ifdef VKU_USE_SHADERC
[[nodiscard]] constexpr auto getShaderKind(VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage) -> shaderc_shader_kind {
    switch (stage) {
        case VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eVertex:
            return shaderc_glsl_vertex_shader;
        case VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eTessellationControl:
            return shaderc_glsl_tess_control_shader;
        case VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eTessellationEvaluation:
            return shaderc_glsl_tess_evaluation_shader;
        case VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eGeometry:
            return shaderc_glsl_geometry_shader;
        case VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eFragment:
            return shaderc_glsl_fragment_shader;
        case VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eCompute:
            return shaderc_glsl_compute_shader;
        default:
            throw std::runtime_error { std::format("Unsupported shader stage: {}", to_string(stage)) };
    }
}
#endif

namespace vku {
    export struct Shader {
        std::span<const std::uint32_t> code;
        VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage;
        const VULKAN_HPP_NAMESPACE::SpecializationInfo *pSpecializationInfo;
        const char *entryPoint = "main";

        [[nodiscard]] static auto fromSpirvFile(
            const std::filesystem::path &path,
            VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage,
            const VULKAN_HPP_NAMESPACE::SpecializationInfo *pSpecializationInfo [[clang::lifetimebound]] = nullptr,
            const char *entryPoint = "main"
        ) -> RefHolder<Shader, std::vector<std::byte>> {
            return {
                [=](std::span<const std::byte> code) {
                    return Shader { as<const std::uint32_t>(code), stage, pSpecializationInfo, entryPoint };
                },
                loadFileAsBinary(path),
            };
        }

#ifdef VKU_USE_SHADERC
        [[nodiscard]] static auto fromGLSLString(
            shaderc::Compiler compiler,
            std::string_view glsl,
            VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage,
            const shaderc::CompileOptions &compileOptions,
            const VULKAN_HPP_NAMESPACE::SpecializationInfo *pSpecializationInfo [[clang::lifetimebound]] = nullptr,
            const char *identifier = details::to_string(std::source_location::current()).c_str()
        ) -> RefHolder<Shader, std::vector<std::uint32_t>> {
            const auto compilationResult = compiler.CompileGlslToSpv(glsl.data(), glsl.size(), getShaderKind(stage), identifier, "main", compileOptions);
            if (compilationResult.GetCompilationStatus() != shaderc_compilation_status_success) {
                throw std::runtime_error { std::format("Failed to compile shader: {}", compilationResult.GetErrorMessage()) };
            }

            return {
                [=](std::span<const std::uint32_t> code) {
                    return Shader { code, stage, pSpecializationInfo };
                },
                compilationResult | std::ranges::to<std::vector>(),
            };
        }

        [[nodiscard]] static auto fromGLSLFile(
            shaderc::Compiler compiler,
            const std::filesystem::path &glslPath,
            VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage,
            const shaderc::CompileOptions &compileOptions,
            const VULKAN_HPP_NAMESPACE::SpecializationInfo *pSpecializationInfo [[clang::lifetimebound]] = nullptr
        ) -> RefHolder<Shader, std::vector<std::uint32_t>> {
            const std::vector glsl = loadFileAsBinary(glslPath);
            const auto compilationResult = compiler.CompileGlslToSpv(reinterpret_cast<const char*>(glsl.data()), glsl.size(), getShaderKind(stage), PATH_C_STR(glslPath), "main", compileOptions);
            if (compilationResult.GetCompilationStatus() != shaderc_compilation_status_success) {
                throw std::runtime_error { std::format("Failed to compile shader: {}", compilationResult.GetErrorMessage()) };
            }

            return {
                [=](std::span<const std::uint32_t> code) {
                    return Shader { code, stage, pSpecializationInfo };
                },
                compilationResult | std::ranges::to<std::vector>(),
            };
        }
#endif
    };
}