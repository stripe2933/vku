#pragma once

#include <functional>
#include <variant>

#include <vulkan/vulkan_raii.hpp>

#include "details/concepts.hpp"
#include "details/ranges.hpp"

namespace vku {
namespace details {
    struct nofield{};
}

    template <typename QueueFamilyIndices, typename Queues> requires
        std::constructible_from<Queues, vk::Device, const QueueFamilyIndices&>
        && requires(const QueueFamilyIndices &queueFamilyIndices) {
            { Queues::getDeviceQueueCreateInfos(queueFamilyIndices) } -> std::ranges::contiguous_range;
        }
    class Gpu {
    public:
        struct DefaultQueueFamilyIndicesGetter {
            [[nodiscard]] auto operator()(
                vk::PhysicalDevice physicalDevice
            ) const -> QueueFamilyIndices {
                return QueueFamilyIndices { physicalDevice };
            }
        };

        struct DefaultPhysicalDeviceRater {
            std::span<const char*> requiredExtensions;
            std::variant<const details::nofield*, const vk::PhysicalDeviceFeatures*> physicalDeviceFeatures;
            std::function<QueueFamilyIndices(vk::PhysicalDevice)> queueFamilyIndicesGetter = DefaultQueueFamilyIndicesGetter{};

            [[nodiscard]] auto operator()(
                vk::PhysicalDevice physicalDevice
            ) const -> std::uint32_t {
                // Check if given physical device supports the required device extensions and features.
                if (!std::ranges::all_of(requiredExtensions, [&, availableExtensions = physicalDevice.enumerateDeviceExtensionProperties()](const char *extensionName) {
                    return std::ranges::any_of(availableExtensions | std::views::transform(&vk::ExtensionProperties::extensionName), [extensionName](const auto &availableExtension) {
                        return std::strcmp(availableExtension.data(), extensionName) == 0;
                    });
                })) {
                    return 0U;
                }

                if (auto *pFeatures = get_if<const vk::PhysicalDeviceFeatures*>(&physicalDeviceFeatures); pFeatures) {
                    const vk::PhysicalDeviceFeatures availableFeatures = physicalDevice.getFeatures();

                    static_assert(sizeof(vk::PhysicalDeviceFeatures) % sizeof(vk::Bool32) == 0,
                        "vk::PhysicalDeviceFeatures must be only consisted of vk::Bool32");
                    const auto* const pFeatureEnables = reinterpret_cast<const vk::Bool32*>(&availableFeatures),
                                      *pFeatureRequests = reinterpret_cast<const vk::Bool32*>(*pFeatures);
                    if (std::ranges::any_of(std::views::iota(std::size_t { 0 }, sizeof(vk::PhysicalDeviceFeatures) / sizeof(vk::Bool32)), [=](std::size_t featureIndex) {
                        // Find a feature that is requested but not available.
                        return pFeatureRequests[featureIndex] && !pFeatureEnables[featureIndex];
                    })) {
                        return 0U;
                    }
                }

                // Check if given physical device has required queue families.
                try {
                    const QueueFamilyIndices queueFamilyIndices = queueFamilyIndicesGetter(physicalDevice);
                }
                catch (...) {
                    return 0U;
                }

                // Rate physical device based on its properties.
                std::uint32_t score = 0;
                const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
                if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                    score += 1000;
                }
                score += properties.limits.maxImageDimension2D;
                return score;
            }
        };

        template <details::tuple_like PNextsTuple = std::tuple<>>
        struct Config {
            std::vector<const char*> extensions;
            // If PNextsTuple has vk::PhysicalDeviceFeatures2 alternative, then physicalDeviceFeatures field set to details::nofield to pretend it is not exists.
            // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDeviceCreateInfo.html#VUID-VkDeviceCreateInfo-pNext-00373
            [[no_unique_address]] std::conditional_t<details::alternative_of<vk::PhysicalDeviceFeatures2, PNextsTuple>, details::nofield, vk::PhysicalDeviceFeatures> physicalDeviceFeatures;
            std::function<QueueFamilyIndices(vk::PhysicalDevice)> queueFamilyIndicesGetter = DefaultQueueFamilyIndicesGetter{};
            std::function<std::uint32_t(vk::PhysicalDevice)> physicalDeviceRater = DefaultPhysicalDeviceRater { extensions, &physicalDeviceFeatures, queueFamilyIndicesGetter };
            PNextsTuple pNexts = std::tuple{};
        };

        vk::raii::PhysicalDevice physicalDevice;
        QueueFamilyIndices queueFamilyIndices;
        vk::raii::Device device;
        Queues queues { *device, queueFamilyIndices };

        template <typename... ConfigArgs>
        explicit Gpu(
            const vk::raii::Instance &instance,
            Config<ConfigArgs...> config = Config<>{}
        ) : physicalDevice { selectPhysicalDevice(instance, config) },
            queueFamilyIndices { std::invoke(config.queueFamilyIndicesGetter, *physicalDevice) },
            device { createDevice(config) } {
#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
            VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);
#endif
        }

        template <typename... ConfigArgs>
        [[nodiscard]] auto selectPhysicalDevice(
            const vk::raii::Instance &instance,
            const Config<ConfigArgs...> &config
        ) const -> vk::raii::PhysicalDevice {
            const std::vector physicalDevices = instance.enumeratePhysicalDevices();
            vk::raii::PhysicalDevice bestPhysicalDevice
                = *std::ranges::max_element(physicalDevices, {}, [&](const vk::raii::PhysicalDevice &physicalDevice) {
                    return std::invoke(config.physicalDeviceRater, *physicalDevice);
                });
            if (std::invoke(config.physicalDeviceRater, *bestPhysicalDevice) == 0U) {
                throw std::runtime_error { "No adequate physical device" };
            }

            return bestPhysicalDevice;
        }

        template <typename... ConfigArgs>
        [[nodiscard]] auto createDevice(
            Config<ConfigArgs...> &config
        ) const -> vk::raii::Device {
#if __APPLE__
            config.extensions.push_back(vk::KHRPortabilitySubsetExtensionName);
#endif

            const auto queueCreateInfos = Queues::getDeviceQueueCreateInfos(queueFamilyIndices);
            return { physicalDevice, std::apply([&](const auto &...args) {
                return vk::StructureChain { vk::DeviceCreateInfo {
                    {},
                    queueCreateInfos,
                    {},
                    config.extensions,
                    [&] -> const vk::PhysicalDeviceFeatures* {
                        return std::convertible_to<decltype(config.physicalDeviceFeatures), vk::PhysicalDeviceFeatures>
                            ? &config.physicalDeviceFeatures
                            : nullptr;
                    }()
                }, args... };
            }, config.pNexts).get() };
        }
    };
}