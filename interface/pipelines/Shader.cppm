/** @file pipelines/Shader.cppm
 */

module;

#include <cassert>
#include <cerrno>

#ifdef VKU_USE_SHADERC
#include <shaderc/shaderc.hpp>
#endif
#include <vulkan/vulkan_hpp_macros.hpp>

#include <lifetimebound.hpp>

export module vku:pipelines.Shader;

import std;
#ifdef VKU_USE_SHADERC
import vku.details;
#endif
export import vulkan_hpp;
import :utils.RefHolder;

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
    /**
     * @brief A lightweight representation of SPIR-V shader code and additional information (stage, specialization info, and entry point).
     */
    export struct Shader {
        /**
         * @brief SPIR-V code.
         */
        std::span<const std::uint32_t> code;

        VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage;

        /**
         * @brief The pointer to the specialization info to be used.
         *
         * If <tt>nullptr</tt>, no specialization is used.
         * @warning If passed, its lifetime must be tied to its actual usage (passed to <tt>vk::create{Graphics,Compute}Pipeline</tt>).
         */
        const VULKAN_HPP_NAMESPACE::SpecializationInfo *pSpecializationInfo;

        const char *entryPoint = "main";

        /**
         * @brief Create Shader from compiled SPIR-V file.
         * @param path Path to the SPIR-V file.
         * @param stage The stage of the shader.
         * @param pSpecializationInfo The specialization info of the shader. If <tt>nullptr</tt>, no specialization is used.
         * @param entryPoint The entry point of the shader. Default is <tt>"main"</tt>.
         * @return A RefHolder of the Shader.
         * @warning If \p pSpecializationInfo passed, its lifetime must be tied to its actual usage (passed to <tt>vk::create{Graphics,Compute}Pipeline</tt>).
         */
        [[nodiscard]] static RefHolder<Shader, std::vector<std::byte>> fromSpirvFile(
            const std::filesystem::path &path,
            VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage,
            const VULKAN_HPP_NAMESPACE::SpecializationInfo *pSpecializationInfo LIFETIMEBOUND = nullptr,
            const char *entryPoint = "main"
        ) {
            return {
                [=](std::span<const std::byte> code) {
                    return Shader { as<const std::uint32_t>(code), stage, pSpecializationInfo, entryPoint };
                },
                loadFileAsBinary(path),
            };
        }

#ifdef VKU_USE_SHADERC
        /**
         * @brief Create Shader from GLSL string.
         *
         * Compile GLSL string into SPIR-V code in runtime using shaderc. You can specify the several compile options
         * (e.g., target environment, optimization level, etc.) by passing the appropriate \p compileOptions struct,
         * and also \p identifier for shader identification.
         *
         * If you're looking for directly load the GLSL file and process it, use vku::Shader::fromGLSLFile instead.
         *
         * @param compiler The shaderc compiler.
         * @param glsl The GLSL string.
         * @param stage The stage of the shader. Must be a kind of either vertex, tessellation control, tessellation evaluation, geometry, fragment, or compute.
         * @param compileOptions The compile options.
         * @param pSpecializationInfo The specialization info of the shader. If <tt>nullptr</tt>, no specialization is used.
         * @param identifier The identifier of the shader. If not specified, it will use the function caller's source location.
         * @return A RefHolder of the Shader.
         * @throw std::runtime_error If the compilation failed.
         * @warning If \p pSpecializationInfo passed, its lifetime must be tied to its actual usage (passed to <tt>vk::create{Graphics,Compute}Pipeline</tt>).
         * @note This function is available only if the macro <tt>VKU_USE_SHADERC</tt> is defined.
         */
        [[nodiscard]] static RefHolder<Shader, std::vector<std::uint32_t>> fromGLSLString(
            shaderc::Compiler compiler,
            std::string_view glsl,
            VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage,
            const shaderc::CompileOptions &compileOptions,
            const VULKAN_HPP_NAMESPACE::SpecializationInfo *pSpecializationInfo LIFETIMEBOUND = nullptr,
            const char *identifier = details::to_string(std::source_location::current()).c_str()
        ) {
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

        /**
         * @brief Create Shader from GLSL file.
         *
         * Load GLSL file and compile it into SPIR-V code in runtime using shaderc. You can specify the several compile
         * options (e.g., target environment, optimization level, etc.) by passing the appropriate \p compileOptions struct,
         * and also \p identifier for shader identification.
         *
         * If you're looking for compile the already existing GLSL string, use vku::Shader::fromGLSLString instead.
         *
         * @param compiler The shaderc compiler.
         * @param glslPath The path to the GLSL file.
         * @param stage The stage of the shader. Must be a kind of either vertex, tessellation control, tessellation evaluation, geometry, fragment, or compute.
         * @param compileOptions The compile options.
         * @param pSpecializationInfo The specialization info of the shader. If <tt>nullptr</tt>, no specialization is used.
         * @return A RefHolder of the Shader.
         * @throw std::runtime_error If the compilation failed.
         * @warning If \p pSpecializationInfo passed, its lifetime must be tied to its actual usage (passed to <tt>vk::create{Graphics,Compute}Pipeline</tt>).
         * @note This function is available only if the macro <tt>VKU_USE_SHADERC</tt> is defined.
         */
        [[nodiscard]] static RefHolder<Shader, std::vector<std::uint32_t>> fromGLSLFile(
            shaderc::Compiler compiler,
            const std::filesystem::path &glslPath,
            VULKAN_HPP_NAMESPACE::ShaderStageFlagBits stage,
            const shaderc::CompileOptions &compileOptions,
            const VULKAN_HPP_NAMESPACE::SpecializationInfo *pSpecializationInfo LIFETIMEBOUND = nullptr
        ) {
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