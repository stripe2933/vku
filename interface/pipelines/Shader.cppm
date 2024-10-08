module;

#include <cerrno>
#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <optional>
#include <ranges>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#endif

#ifdef VKU_USE_SHADERC
#include <shaderc/shaderc.hpp>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:pipelines.Shader;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vulkan_hpp;
import :details.to_string;

namespace vku {
    export struct Shader {
        std::vector<std::uint32_t> code;
        VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage;
        const char *entryPoint;
        std::optional<VULKAN_HPP_NAMESPACE::SpecializationInfo> specializationInfo;

        // --------------------
        // Constructors.
        // --------------------

        /**
         * Construct <tt>Shader</tt> from existing SPIR-V shader code vector.
         * @param code SPIR-V shader code. This will be moved to the struct.
         * @param stage Shader stage.
         * @param entryPoint Entry point function name (default: "main").
         */
        Shader(std::vector<std::uint32_t> code, VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage, const char *entryPoint = "main");

        /**
         * Construct <tt>Shader</tt> from existing SPIR-V shader code vector.
         * @param code SPIR-V shader code. This will be moved to the struct.
         * @param stage Shader stage.
         * @param specializationInfo Specialization info that are used for specialization constants.
         * @param entryPoint Entry point function name (default: "main").
         * @note \p specializationInfo entries must live longer than the destination pipeline creation finish.
         */
        Shader(
            std::vector<std::uint32_t> code, 
            VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage, 
            const VULKAN_HPP_NAMESPACE::SpecializationInfo &specializationInfo [[clang::lifetimebound]], 
            const char *entryPoint = "main"
        ) : Shader { std::move(code), stage, entryPoint } {
            this->specializationInfo.emplace(specializationInfo);
        }

        /**
         * Construct <tt>Shader</tt> from compiled SPIR-V file.
         * @param path Path to the compiled SPIR-V file.
         * @param stage Shader stage.
         * @param entryPoint Entry point function name (default: "main").
         */
        Shader(const std::filesystem::path &path, VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage, const char *entryPoint = "main");

        /**
         * Construct <tt>Shader</tt> from compiled SPIR-V file.
         * @param path Path to the compiled SPIR-V file.
         * @param stage Shader stage.
         * @param specializationInfo Specialization info that are used for specialization constants.
         * @param entryPoint Entry point function name (default: "main").
         * @note \p specializationInfo entries must live longer than the destination pipeline creation finish.
         */
        Shader(
            const std::filesystem::path &path, 
            VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage, 
            const VULKAN_HPP_NAMESPACE::SpecializationInfo &specializationInfo [[clang::lifetimebound]], 
            const char *entryPoint = "main"
        ) : Shader { path, stage, entryPoint } {
            this->specializationInfo.emplace(specializationInfo);
        }

#ifdef VKU_USE_SHADERC
        /**
         * Construct <tt>Shader</tt> from GLSL source code.
         * @param compiler Shader compiler instance.
         * @param glsl GLSL source code.
         * @param stage Shader stage. Only vertex/tessellation control/tessellation evaluation/geometry/fragment/compute shaders are supported.
         * @param entryPoint Entry point function name (default: "main").
         * @param identifier Identifier for the shader (default: current source location).
         * @throw std::runtime_error If \p stage is unsupported.
         * @throw std::runtime_error If shader compilation failed.
         */
        Shader(
            const shaderc::Compiler &compiler, 
            std::string_view glsl, 
            VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage, 
            const char *entryPoint = "main", 
            const char *identifier = details::to_string(std::source_location::current()).c_str());
        
        /**
         * Construct <tt>Shader</tt> from GLSL source code.
         * @param compiler Shader compiler instance.
         * @param glsl GLSL source code.
         * @param stage Shader stage. Only vertex/tessellation control/tessellation evaluation/geometry/fragment/compute shaders are supported.
         * @param specializationInfo Specialization info that are used for specialization constants.
         * @param entryPoint Entry point function name (default: "main").
         * @param identifier Identifier for the shader (default: current source location).
         * @throw std::runtime_error If \p stage is unsupported.
         * @throw std::runtime_error If shader compilation failed.
         * @note \p specializationInfo entries must live longer than the destination pipeline creation finish.
         */
        Shader(
            const shaderc::Compiler &compiler, 
            std::string_view glsl, 
            VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage, 
            const VULKAN_HPP_NAMESPACE::SpecializationInfo &specializationInfo [[clang::lifetimebound]], 
            const char *entryPoint = "main", 
            const char *identifier = details::to_string(std::source_location::current()).c_str()
        ) : Shader { compiler, glsl, stage, entryPoint, identifier } {
            this->specializationInfo.emplace(specializationInfo);
        }
#endif
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
    VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage,
    const char *entryPoint
) : code { std::move(code) },
    stage { stage },
    entryPoint { entryPoint } { }

vku::Shader::Shader(
    const std::filesystem::path &path,
    VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage,
    const char *entryPoint
) : code { loadFileAsBinary(path) },
    stage { stage },
    entryPoint { entryPoint } { }

#ifdef VKU_USE_SHADERC
[[nodiscard]] constexpr auto getShaderKind(
    VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage
) -> shaderc_shader_kind {
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
            throw std::runtime_error { std::format("Unsupported shader stage: {}", details::to_string(stage)) };
    }
}

vku::Shader::Shader(
    const shaderc::Compiler &compiler,
    std::string_view glsl,
    VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage,
    const char *entryPoint,
    const char *identifier
) : stage { stage },
    entryPoint { entryPoint } {
    shaderc::CompileOptions compileOptions;
    // TODO: parameter option for selecting Vulkan version?
    compileOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
#ifdef NDEBUG
    compileOptions.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif

    const auto result = compiler.CompileGlslToSpv(
        glsl.data(), glsl.size(),
        getShaderKind(stage),
        entryPoint, identifier, compileOptions);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        throw std::runtime_error { std::format("Failed to compile shader: {}", result.GetErrorMessage()) };
    }

    code = result | std::ranges::to<std::vector>();
}
#endif