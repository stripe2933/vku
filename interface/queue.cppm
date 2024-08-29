module;

#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <optional>
#include <span>

#ifdef _MSC_VER
#include <compare>
#include <string_view>
#endif
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:queue;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vulkan_hpp;

namespace vku {
    /**
     * Get compute capable queue family index.
     * @param queueFamilyProperties Queue family properties. Could be enumerated by <tt>vk::PhysicalDevice::getQueueFamilyProperties</tt>.
     * @return The index of the compute capable queue family, or <tt>std::nullopt</tt> if not found.
     */
    export
    [[nodiscard]] auto getComputeQueueFamily(
        std::span<const VULKAN_HPP_NAMESPACE::QueueFamilyProperties> queueFamilyProperties
    ) noexcept -> std::optional<std::uint32_t> {
        for (std::uint32_t i = 0; const VULKAN_HPP_NAMESPACE::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & VULKAN_HPP_NAMESPACE::QueueFlagBits::eCompute) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    /**
     * Get compute specialized (queue flag without graphics) queue family index.
     * @param queueFamilyProperties Queue family properties. Could be enumerated by <tt>vk::PhysicalDevice::getQueueFamilyProperties</tt>.
     * @return The index of the compute specialized queue family, or <tt>std::nullopt</tt> if not found.
     */
    export
    [[nodiscard]] auto getComputeSpecializedQueueFamily(
        std::span<const VULKAN_HPP_NAMESPACE::QueueFamilyProperties> queueFamilyProperties
    ) noexcept -> std::optional<std::uint32_t> {
        for (std::uint32_t i = 0; const VULKAN_HPP_NAMESPACE::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & VULKAN_HPP_NAMESPACE::QueueFlagBits::eCompute && !(properties.queueFlags & VULKAN_HPP_NAMESPACE::QueueFlagBits::eGraphics)) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    /**
     * Get graphics capable queue family index.
     * @param queueFamilyProperties Queue family properties. Could be enumerated by <tt>vk::PhysicalDevice::getQueueFamilyProperties</tt>.
     * @return The index of the graphics capable queue family, or <tt>std::nullopt</tt> if not found.
     */
    export
    [[nodiscard]] auto getGraphicsQueueFamily(
        std::span<const VULKAN_HPP_NAMESPACE::QueueFamilyProperties> queueFamilyProperties
    ) noexcept -> std::optional<std::uint32_t> {
        for (std::uint32_t i = 0; const VULKAN_HPP_NAMESPACE::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & VULKAN_HPP_NAMESPACE::QueueFlagBits::eGraphics) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    /**
     * Get transfer specialized (queue flag without both compute and graphics) queue family index.
     * @param queueFamilyProperties Queue family properties. Could be enumerated by <tt>vk::PhysicalDevice::getQueueFamilyProperties</tt>.
     * @return The index of the transfer specialized queue family, or <tt>std::nullopt</tt> if not found.
     */
    export
    [[nodiscard]] auto getTransferSpecializedQueueFamily(
        std::span<const VULKAN_HPP_NAMESPACE::QueueFamilyProperties> queueFamilyProperties
    ) noexcept -> std::optional<std::uint32_t> {
        for (std::uint32_t i = 0; const VULKAN_HPP_NAMESPACE::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & VULKAN_HPP_NAMESPACE::QueueFlagBits::eTransfer && !(properties.queueFlags & (VULKAN_HPP_NAMESPACE::QueueFlagBits::eCompute | VULKAN_HPP_NAMESPACE::QueueFlagBits::eGraphics))) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    /**
     * Get present capable queue family index.
     * @param physicalDevice Vulkan physical device to get queue families from.
     * @param surface Surface to test for presentation support.
     * @param queueFamilyCount Number of queue families, which is identical to <tt>physicalDevice.getQueueFamilyProperties().size()</tt>.
     * @return The index of the present capable queue family, or <tt>std::nullopt</tt> if not found.
     */
    export
    [[nodiscard]] auto getPresentQueueFamily(
        VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice,
        VULKAN_HPP_NAMESPACE::SurfaceKHR surface,
        std::uint32_t queueFamilyCount
    ) -> std::optional<std::uint32_t> {
        for (std::uint32_t i = 0; i < queueFamilyCount; ++i) {
            if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
                return i;
            }
        }
        return std::nullopt;
    }

    /**
     * Get both graphics and present capable queue family index.
     * @param physicalDevice Vulkan physical device to get queue families from.
     * @param surface Surface to test for presentation support.
     * @param queueFamilyProperties Queue family properties, which is identical to <tt>physicalDevice.getQueueFamilyProperties()</tt>.
     * @return The index of the both graphics and present capable queue family, or <tt>std::nullopt</tt> if not found.
     * @note
     * This is useful for the common case of rendering to a window, because it doesn't requires the explicit queue family
     * ownership transfer between graphics and present queues.
     */
    export
    [[nodiscard]] auto getGraphicsPresentQueueFamily(
        VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice,
        VULKAN_HPP_NAMESPACE::SurfaceKHR surface,
        std::span<const VULKAN_HPP_NAMESPACE::QueueFamilyProperties> queueFamilyProperties
    ) -> std::optional<std::uint32_t> {
        for (std::uint32_t i = 0; const VULKAN_HPP_NAMESPACE::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & VULKAN_HPP_NAMESPACE::QueueFlagBits::eGraphics && physicalDevice.getSurfaceSupportKHR(i, surface)) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    /**
     * Get both compute and present capable queue family index.
     * @param physicalDevice Vulkan physical device to get queue families from.
     * @param surface Surface to test for presentation support.
     * @param queueFamilyProperties Queue family properties, which is identical to <tt>physicalDevice.getQueueFamilyProperties()</tt>.
     * @return The index of the both compute and present capable queue family, or <tt>std::nullopt</tt> if not found.
     * @note
     * This is useful for compute based window rendering, because it doesn't requires the explicit queue family ownership
     * transfer between compute and present queues.
     */
    export
    [[nodiscard]] auto getComputePresentQueueFamily(
        VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice,
        VULKAN_HPP_NAMESPACE::SurfaceKHR surface,
        std::span<const VULKAN_HPP_NAMESPACE::QueueFamilyProperties> queueFamilyProperties
    ) -> std::optional<std::uint32_t> {
        for (std::uint32_t i = 0; const VULKAN_HPP_NAMESPACE::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & VULKAN_HPP_NAMESPACE::QueueFlagBits::eCompute && physicalDevice.getSurfaceSupportKHR(i, surface)) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }
}