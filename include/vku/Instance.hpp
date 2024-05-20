#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "details/concepts.hpp"

namespace vku {
    class Instance {
    public:
        template <details::tuple_like PNextsTuple = std::tuple<>>
        struct Config {
            std::vector<const char*> layers;
            std::vector<const char*> extensions;
            PNextsTuple pNexts = std::tuple<>{};
        };

        vk::raii::Context context;
        vk::raii::Instance instance;

        template <typename... ConfigArgs>
        explicit Instance(
            const vk::ApplicationInfo &applicationInfo,
            Config<ConfigArgs...> config = {}
        ) : instance { createInstance(applicationInfo, config) } {
#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
            VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
#endif
        }

    private:
        template <typename... ConfigArgs>
        [[nodiscard]] auto createInstance(
            const vk::ApplicationInfo &applicationInfo,
            Config<ConfigArgs...> &config
        ) const -> vk::raii::Instance {
#ifndef NDEBUG
            config.layers.emplace_back("VK_LAYER_KHRONOS_validation");
#endif
#if __APPLE__
#if VKU_VK_VERSION < 1001000
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            config.extensions.emplace_back(vk::KHRGetPhysicalDeviceProperties2ExtensionName);
#pragma GCC diagnostic pop
#endif
            config.extensions.emplace_back(vk::KHRPortabilityEnumerationExtensionName);
#endif

            return { context, std::apply([&](const auto &...pNexts) {
                return vk::StructureChain { vk::InstanceCreateInfo {
#if __APPLE__
                    vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
#else
                    {},
#endif
                    &applicationInfo,
                    config.layers,
                    config.extensions,
                }, pNexts... };
            }, config.pNexts).get() };
        }
    };
}