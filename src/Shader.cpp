#include <vku/Shader.hpp>

#include <fstream>

auto vku::Shader::readCode(const std::filesystem::path &path) -> std::vector<std::uint32_t> {
    std::ifstream file { path, std::ios::ate | std::ios::binary };
    if (!file.is_open()) {
        throw std::runtime_error { std::format("Failed to open file {} ({})", path.string(), strerror(errno)) };
    }

    const std::size_t fileSizeInBytes = file.tellg();
    std::vector<std::uint32_t> buffer(fileSizeInBytes / sizeof(std::uint32_t));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSizeInBytes);

    return buffer;
}
