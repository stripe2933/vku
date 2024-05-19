#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace vku {
    class GlfwWindow {
    public:
        GLFWwindow *window;
        vk::raii::SurfaceKHR surface;

        GlfwWindow(int width, int height, const char *title, const vk::raii::Instance &instance);
        GlfwWindow(const GlfwWindow&) = delete;
        GlfwWindow(GlfwWindow &&src) noexcept;
        auto operator=(const GlfwWindow&) -> GlfwWindow& = delete;
        auto operator=(GlfwWindow &&src) noexcept -> GlfwWindow&;

        virtual ~GlfwWindow();

        virtual auto onSizeCallback(glm::ivec2 size) -> void { }
        virtual auto onFramebufferSizeCallback(glm::ivec2 size) -> void { }
        virtual auto onContentScaleCallback(glm::vec2 scale) -> void { }
        virtual auto onKeyCallback(int key, int scancode, int action, int mods) -> void { }
        virtual auto onCharCallback(unsigned int codepoint) -> void { }
        virtual auto onCursorPosCallback(glm::dvec2 position) -> void { }
        virtual auto onCursorEnterCallback(int entered) -> void { }
        virtual auto onMouseButtonCallback(int button, int action, int mods) -> void { }
        virtual auto onScrollCallback(glm::dvec2 offset) -> void { }
        virtual auto onDropCallback(std::span<const char*> paths) -> void { }

        [[nodiscard]] auto getSize() const -> glm::ivec2;
        [[nodiscard]] auto getFramebufferSize() const -> glm::ivec2;
        [[nodiscard]] auto getCursorPos() const -> glm::dvec2;
        [[nodiscard]] auto getContentScale() const -> glm::vec2;

        [[nodiscard]] static auto getInstanceExtensions() -> std::vector<const char*>;

    private:
        [[nodiscard]] auto createSurface(const vk::raii::Instance &instance) const -> vk::raii::SurfaceKHR;
    };
}