#include <vku/GlfwWindow.hpp>

#include <format>

vku::GlfwWindow::GlfwWindow(int width, int height, const char *title, const vk::raii::Instance &instance): window {
        glfwCreateWindow(width, height, title, nullptr, nullptr)
    },
    surface { createSurface(instance) } {
    if (!window) {
        const char *error;
        const int code = glfwGetError(&error);
        throw std::runtime_error { std::format("Failed to create GLFW window: {} (error code {})", error, code) };
    }

    glfwSetWindowUserPointer(window, this);

    glfwSetWindowSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))
            ->onSizeCallback({ width, height });
    });
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))
            ->onFramebufferSizeCallback({ width, height });
    });
    glfwSetWindowContentScaleCallback(window, [](GLFWwindow *window, float xscale, float yscale) {
        static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))
            ->onContentScaleCallback({ xscale, yscale });
    });
    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))
            ->onKeyCallback(key, scancode, action, mods);
    });
    glfwSetCharCallback(window, [](GLFWwindow *window, unsigned int codepoint) {
        static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))
            ->onCharCallback(codepoint);
    });
    glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos) {
        static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))
            ->onCursorPosCallback({ xpos, ypos });
    });
    glfwSetCursorEnterCallback(window, [](GLFWwindow *window, int entered) {
        static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))
            ->onCursorEnterCallback(entered);
    });
    glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods) {
        static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))
            ->onMouseButtonCallback(button, action, mods);
    });
    glfwSetScrollCallback(window, [](GLFWwindow *window, double xoffset, double yoffset) {
        static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))
            ->onScrollCallback({ xoffset, yoffset });
    });
    glfwSetDropCallback(window, [](GLFWwindow *window, int count, const char **paths) {
        static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))
            ->onDropCallback({ paths, static_cast<std::size_t>(count) });
    });
}

vku::GlfwWindow::GlfwWindow(GlfwWindow &&src) noexcept
    : window { std::exchange(src.window, nullptr) },
      surface { std::move(src.surface) } { }

auto vku::GlfwWindow::operator=(GlfwWindow &&src) noexcept -> GlfwWindow & {
    if (window) {
        glfwDestroyWindow(window);
    }

    window = std::exchange(src.window, nullptr);
    surface = std::move(src.surface);
    return *this;
}

vku::GlfwWindow::~GlfwWindow() {
    if (window) {
        glfwDestroyWindow(window);
    }
}

auto vku::GlfwWindow::getSize() const -> glm::ivec2 {
    glm::ivec2 size;
    glfwGetWindowSize(window, &size.x, &size.y);
    return size;
}

auto vku::GlfwWindow::getFramebufferSize() const -> glm::ivec2 {
    glm::ivec2 size;
    glfwGetFramebufferSize(window, &size.x, &size.y);
    return size;
}

auto vku::GlfwWindow::getCursorPos() const -> glm::dvec2 {
    glm::dvec2 pos;
    glfwGetCursorPos(window, &pos.x, &pos.y);
    return pos;
}

auto vku::GlfwWindow::getContentScale() const -> glm::vec2 {
    glm::vec2 scale;
    glfwGetWindowContentScale(window, &scale.x, &scale.y);
    return scale;
}

auto vku::GlfwWindow::getInstanceExtensions() -> std::vector<const char*> {
    std::uint32_t count;
    const char **extensions = glfwGetRequiredInstanceExtensions(&count);
    return { extensions, extensions + count };
}

auto vku::GlfwWindow::createSurface(const vk::raii::Instance &instance) const -> vk::raii::SurfaceKHR {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(*instance, window, nullptr, &surface) != VK_SUCCESS) {
        const char *error;
        const int code = glfwGetError(&error);
        throw std::runtime_error { std::format("Failed to create window surface: {} (error code {})", error, code) };
    }
    return { instance, surface };
}
