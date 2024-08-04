module;

#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <algorithm>
#include <concepts>
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

#define CHECK_FEATURE(feature) if (pPhysicalDeviceFeatures->feature && !availableFeatures.feature) { unavailableFeatures.push_back(#feature); }

namespace vku {
    export template <typename QueueFamilies, std::constructible_from<vk::Device, const QueueFamilies&> Queues> requires
        requires(vk::PhysicalDevice physicalDevice, const QueueFamilies &queueFamilies) {
            { Queues::getCreateInfos(physicalDevice, queueFamilies).get() } -> ranges::contiguous_range_of<vk::DeviceQueueCreateInfo>;
        }
    class Gpu {
        [[nodiscard]] static auto getQueueFamilies(vk::PhysicalDevice physicalDevice) noexcept -> QueueFamilies {
            return QueueFamilies { physicalDevice };
        }

    public:
        struct DefaultPhysicalDeviceRater {
            bool verbose;
            std::function<QueueFamilies(vk::PhysicalDevice)> queueFamilyGetter;
            std::span<const char* const> deviceExtensions;
            const vk::PhysicalDeviceFeatures *pPhysicalDeviceFeatures = nullptr;

            [[nodiscard]] auto operator()(vk::PhysicalDevice physicalDevice) const -> std::uint32_t {
                const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
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
                    | std::views::transform(&vk::ExtensionProperties::extensionName)
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
                const vk::PhysicalDeviceFeatures availableFeatures = physicalDevice.getFeatures();
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
                if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
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
            static constexpr bool hasPhysicalDeviceFeatures = !concepts::one_of<vk::PhysicalDeviceFeatures2, DevicePNexts...>;

            bool verbose = false;
            std::vector<const char*> deviceExtensions = {};
            [[no_unique_address]]
            std::conditional_t<hasPhysicalDeviceFeatures, vk::PhysicalDeviceFeatures, std::monostate> physicalDeviceFeatures = {};
            std::function<QueueFamilies(vk::PhysicalDevice)> queueFamilyGetter = &getQueueFamilies;
            std::function<std::uint32_t(vk::PhysicalDevice)> physicalDeviceRater
                = DefaultPhysicalDeviceRater { verbose, queueFamilyGetter, deviceExtensions, hasPhysicalDeviceFeatures ? &physicalDeviceFeatures : nullptr };
            std::tuple<DevicePNexts...> devicePNexts = {};
            vma::AllocatorCreateFlags allocatorCreateFlags = {};
            std::uint32_t apiVersion = vk::makeApiVersion(0, 1, 0, 0);
        };

        vk::raii::PhysicalDevice physicalDevice;
        QueueFamilies queueFamilies;
        vk::raii::Device device;
        Queues queues { *device, queueFamilies };
        vma::Allocator allocator;

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
        [[nodiscard]] auto selectPhysicalDevice(
            const vk::raii::Instance &instance,
            const Config<DevicePNexts...> &config
        ) const -> vk::raii::PhysicalDevice {
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
        ) const -> vma::Allocator {
            return vma::createAllocator(vma::AllocatorCreateInfo {
                config.allocatorCreateFlags,
                *physicalDevice, *device,
                {}, {}, {}, {},
#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
                vku::unsafeAddress(vma::VulkanFunctions{
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