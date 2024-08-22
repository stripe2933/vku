module;

#include <version>
#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <algorithm>
#include <concepts>
#include <format>
#include <functional>
#include <iostream>
#include <print>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:Gpu;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vk_mem_alloc_hpp;
export import vulkan_hpp;
import :details;
import :utils;

// #define VMA_HPP_NAMESPACE to vma, if not defined.
#ifndef VMA_HPP_NAMESPACE
#define VMA_HPP_NAMESPACE vma
#endif

#define CHECK_FEATURE(feature) if (pPhysicalDeviceFeatures->feature && !availableFeatures.feature) { unavailableFeatures.push_back(#feature); }

namespace vku {
    export template <typename QueueFamilies, std::constructible_from<VULKAN_HPP_NAMESPACE::Device, const QueueFamilies&> Queues> requires
        requires(VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice, const QueueFamilies &queueFamilies) {
            { Queues::getCreateInfos(physicalDevice, queueFamilies).get() } -> ranges::contiguous_range_of<VULKAN_HPP_NAMESPACE::DeviceQueueCreateInfo>;
        }
    class Gpu {
        [[nodiscard]] static auto getQueueFamilies(VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice) noexcept -> QueueFamilies {
            return QueueFamilies { physicalDevice };
        }

    public:
        struct DefaultPhysicalDeviceRater {
            bool verbose;
            std::function<QueueFamilies(VULKAN_HPP_NAMESPACE::PhysicalDevice)> queueFamilyGetter;
            std::span<const char* const> deviceExtensions;
            const VULKAN_HPP_NAMESPACE::PhysicalDeviceFeatures *pPhysicalDeviceFeatures = nullptr;

            [[nodiscard]] auto operator()(VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice) const -> std::uint32_t {
                const VULKAN_HPP_NAMESPACE::PhysicalDeviceProperties properties = physicalDevice.getProperties();
                const std::string_view deviceName { properties.deviceName.data() };

                // Check queue family availability.
                try {
                    std::ignore = queueFamilyGetter(physicalDevice);
                }
                catch (const std::runtime_error &e) {
                    if (verbose) {
                        std::println(std::cerr, "Physical device \"{}\" rejected because it failed to get the request queue families: {}", deviceName, e.what());
                    }
                    return 0;
                }

                // Check device extension availability.
                const std::vector availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

                constexpr auto toStringView = [](const auto &str) { return std::string_view { str }; };
                std::vector availableExtensionNames
                    = availableExtensions
                    | std::views::transform(&VULKAN_HPP_NAMESPACE::ExtensionProperties::extensionName)
                    | std::views::transform(toStringView)
                    | std::ranges::to<std::vector>();
                std::ranges::sort(availableExtensionNames);

                std::vector deviceExtensionNames
                    = deviceExtensions
                    | std::views::transform(toStringView)
                    | std::ranges::to<std::vector>();
                std::ranges::sort(deviceExtensionNames);

                if (!std::ranges::includes(availableExtensionNames, deviceExtensionNames)) {
                    if (verbose) {
                        std::vector<std::string_view> unavailableExtensions;
                        std::ranges::set_difference(deviceExtensionNames, availableExtensionNames, std::back_inserter(unavailableExtensions));
#if __cpp_lib_format_ranges >= 202207L
                        std::println(std::cerr, "Physical device \"{}\" rejected because it lacks the following device extensions: {::s}", deviceName, unavailableExtensions);
#else
                        std::print(std::cerr, "Physical device \"{}\" rejected because it lacks the following device extensions: [", deviceName);
                        for (std::size_t i = 0; i < unavailableExtensions.size(); ++i) {
                            if (i == unavailableExtensions.size() - 1) {
                                std::println(std::cerr, "{}]", unavailableExtensions[i]);
                            }
                            else {
                                std::print(std::cerr, "{}, ", unavailableExtensions[i]);
                            }
                        }
#endif
                    }
                    return 0;
                }

                // Check physical device feature availability.
                const VULKAN_HPP_NAMESPACE::PhysicalDeviceFeatures availableFeatures = physicalDevice.getFeatures();
                if (pPhysicalDeviceFeatures) {
                    // I hope vk::PhysicalDeviceFeatures struct does not change in the future...
                    std::vector<const char*> unavailableFeatures;
                    CHECK_FEATURE(robustBufferAccess);
                    CHECK_FEATURE(fullDrawIndexUint32);
                    CHECK_FEATURE(imageCubeArray);
                    CHECK_FEATURE(independentBlend);
                    CHECK_FEATURE(geometryShader);
                    CHECK_FEATURE(tessellationShader);
                    CHECK_FEATURE(sampleRateShading);
                    CHECK_FEATURE(dualSrcBlend);
                    CHECK_FEATURE(logicOp);
                    CHECK_FEATURE(multiDrawIndirect);
                    CHECK_FEATURE(drawIndirectFirstInstance);
                    CHECK_FEATURE(depthClamp);
                    CHECK_FEATURE(depthBiasClamp);
                    CHECK_FEATURE(fillModeNonSolid);
                    CHECK_FEATURE(depthBounds);
                    CHECK_FEATURE(wideLines);
                    CHECK_FEATURE(largePoints);
                    CHECK_FEATURE(alphaToOne);
                    CHECK_FEATURE(multiViewport);
                    CHECK_FEATURE(samplerAnisotropy);
                    CHECK_FEATURE(textureCompressionETC2);
                    CHECK_FEATURE(textureCompressionASTC_LDR);
                    CHECK_FEATURE(textureCompressionBC);
                    CHECK_FEATURE(occlusionQueryPrecise);
                    CHECK_FEATURE(pipelineStatisticsQuery);
                    CHECK_FEATURE(vertexPipelineStoresAndAtomics);
                    CHECK_FEATURE(fragmentStoresAndAtomics);
                    CHECK_FEATURE(shaderTessellationAndGeometryPointSize);
                    CHECK_FEATURE(shaderImageGatherExtended);
                    CHECK_FEATURE(shaderStorageImageExtendedFormats);
                    CHECK_FEATURE(shaderStorageImageMultisample);
                    CHECK_FEATURE(shaderStorageImageReadWithoutFormat);
                    CHECK_FEATURE(shaderStorageImageWriteWithoutFormat);
                    CHECK_FEATURE(shaderUniformBufferArrayDynamicIndexing);
                    CHECK_FEATURE(shaderSampledImageArrayDynamicIndexing);
                    CHECK_FEATURE(shaderStorageBufferArrayDynamicIndexing);
                    CHECK_FEATURE(shaderStorageImageArrayDynamicIndexing);
                    CHECK_FEATURE(shaderClipDistance);
                    CHECK_FEATURE(shaderCullDistance);
                    CHECK_FEATURE(shaderFloat64);
                    CHECK_FEATURE(shaderInt64);
                    CHECK_FEATURE(shaderInt16);
                    CHECK_FEATURE(shaderResourceResidency);
                    CHECK_FEATURE(shaderResourceMinLod);
                    CHECK_FEATURE(sparseBinding);
                    CHECK_FEATURE(sparseResidencyBuffer);
                    CHECK_FEATURE(sparseResidencyImage2D);
                    CHECK_FEATURE(sparseResidencyImage3D);
                    CHECK_FEATURE(sparseResidency2Samples);
                    CHECK_FEATURE(sparseResidency4Samples);
                    CHECK_FEATURE(sparseResidency8Samples);
                    CHECK_FEATURE(sparseResidency16Samples);
                    CHECK_FEATURE(sparseResidencyAliased);
                    CHECK_FEATURE(variableMultisampleRate);
                    CHECK_FEATURE(inheritedQueries);

                    if (!unavailableFeatures.empty()) {
                        if (verbose) {
#if __cpp_lib_format_ranges >= 202207L
                            std::println(std::cerr, "Physical device \"{}\" rejected because it lacks the following physical device features: {::s}", deviceName, unavailableFeatures);
#else
                            std::print(std::cerr, "Physical device \"{}\" rejected because it lacks the following physical device features: [", deviceName);
                            for (std::size_t i = 0; i < unavailableFeatures.size(); ++i) {
                                if (i == unavailableFeatures.size() - 1) {
                                    std::println(std::cerr, "{}]", unavailableFeatures[i]);
                                }
                                else {
                                    std::print(std::cerr, "{}, ", unavailableFeatures[i]);
                                }
                            }
#endif
                        }
                        return 0;
                    }
                }

                std::uint32_t score = 0;
                if (properties.deviceType == VULKAN_HPP_NAMESPACE::PhysicalDeviceType::eDiscreteGpu) {
                    score += 1000;
                }

                score += properties.limits.maxImageDimension2D;

                if (verbose) {
                    std::println(std::cerr, "Physical device \"{}\" accepted (score={}).", deviceName, score);
                }
                return score;
            }
        };

        template <typename... DevicePNexts>
        struct Config {
            static constexpr bool hasPhysicalDeviceFeatures = !concepts::one_of<VULKAN_HPP_NAMESPACE::PhysicalDeviceFeatures2, DevicePNexts...>;

            bool verbose = false;
            std::vector<const char*> deviceExtensions = {};
            [[no_unique_address]]
            std::conditional_t<hasPhysicalDeviceFeatures, VULKAN_HPP_NAMESPACE::PhysicalDeviceFeatures, std::monostate> physicalDeviceFeatures = {};
            std::function<QueueFamilies(VULKAN_HPP_NAMESPACE::PhysicalDevice)> queueFamilyGetter = &getQueueFamilies;
            std::function<std::uint32_t(VULKAN_HPP_NAMESPACE::PhysicalDevice)> physicalDeviceRater
                = DefaultPhysicalDeviceRater { verbose, queueFamilyGetter, deviceExtensions, hasPhysicalDeviceFeatures ? &physicalDeviceFeatures : nullptr };
            std::tuple<DevicePNexts...> devicePNexts = {};
            VMA_HPP_NAMESPACE::AllocatorCreateFlags allocatorCreateFlags = {};
            std::uint32_t apiVersion = VULKAN_HPP_NAMESPACE::makeApiVersion(0, 1, 0, 0);
        };

        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::PhysicalDevice physicalDevice;
        QueueFamilies queueFamilies;
        vk::raii::Device device;
        Queues queues { *device, queueFamilies };
        VMA_HPP_NAMESPACE::Allocator allocator;

        template <typename... DevicePNexts>
        explicit Gpu(
            const vk::raii::Instance &instance [[clang::lifetimebound]],
            const Config<DevicePNexts...> &config = {}
        ) : physicalDevice { selectPhysicalDevice(instance, config) },
            queueFamilies { config.queueFamilyGetter(physicalDevice) },
            device { createDevice(config) },
            allocator { createAllocator(instance, config) } { }

        ~Gpu() {
            allocator.destroy();
        }

    private:
        template <typename... DevicePNexts>
        [[nodiscard]] static auto selectPhysicalDevice(
            const vk::raii::Instance &instance,
            const Config<DevicePNexts...> &config
        ) -> vk::raii::PhysicalDevice {
            std::vector physicalDevices = instance.enumeratePhysicalDevices();
            vk::raii::PhysicalDevice bestPhysicalDevice = *std::ranges::max_element(physicalDevices, {}, config.physicalDeviceRater);
            if (config.physicalDeviceRater(*bestPhysicalDevice) == 0) {
                throw std::runtime_error { "No suitable GPU for the application." };
            }
            return bestPhysicalDevice;
        }

        template <typename... PNexts>
        [[nodiscard]] auto createDevice(
            const Config<PNexts...> &config
        ) const -> vk::raii::Device {
            // This have to be here, because after end of the RefHolder::get() expression, queue priorities are
            // destroyed (which makes the pointer to them becomes invalid), but it is still in the std::apply scope.
            const auto queueCreateInfos = Queues::getCreateInfos(*physicalDevice, queueFamilies);
            vk::raii::Device device { physicalDevice, std::apply([&](const auto &...pNexts) {
                const vk::PhysicalDeviceFeatures *pPhysicalDeviceFeatures = nullptr;
                if constexpr (Config<PNexts...>::hasPhysicalDeviceFeatures) {
                    pPhysicalDeviceFeatures = &config.physicalDeviceFeatures;
                }

                /* Note:
                 * Directly returning vk::StructureChain will cause runtime error, because pNexts pointer chain gets
                 * invalidated. Creating non-const result value and returning it works because of the RVO. After C++17,
                 * RVO is guaranteed by the standard. */
                vk::StructureChain createInfo {
                    vk::DeviceCreateInfo {
                        {},
                        queueCreateInfos.get(),
                        {},
                        config.deviceExtensions,
                        pPhysicalDeviceFeatures,
                    },
                    pNexts...,
                };

                return createInfo;
            }, config.devicePNexts).get() };

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
            VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);
#endif
            return device;
        }

        template <typename... DevicePNexts>
        [[nodiscard]] auto createAllocator(
            const vk::raii::Instance &instance,
            const Config<DevicePNexts...> &config
        ) const -> VMA_HPP_NAMESPACE::Allocator {
            return VMA_HPP_NAMESPACE::createAllocator(VMA_HPP_NAMESPACE::AllocatorCreateInfo {
                config.allocatorCreateFlags,
                *physicalDevice, *device,
                {}, {}, {}, {},
#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
                unsafeAddress(VMA_HPP_NAMESPACE::VulkanFunctions{
                    instance.getDispatcher()->vkGetInstanceProcAddr,
                    device.getDispatcher()->vkGetDeviceProcAddr,
                }),
#else
                {},
#endif
                *instance, config.apiVersion,
            });
        }
    };
}