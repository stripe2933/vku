/** @file queue.cppm
 */

module;

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:queue;

import std;
export import vulkan_hpp;

namespace vku {
    /**
     * @brief Retrieve the index of compute capable queue family from \p queueFamilyProperties.
     * @param queueFamilyProperties Queue family properties. Could be enumerated by <tt>vk::PhysicalDevice::getQueueFamilyProperties</tt>.
     * @return The index of the compute capable queue family, or <tt>std::nullopt</tt> if not found.
     */
    export
    [[nodiscard]] std::optional<std::uint32_t> getComputeQueueFamily(
        std::span<const VULKAN_HPP_NAMESPACE::QueueFamilyProperties> queueFamilyProperties
    ) noexcept {
        for (std::uint32_t i = 0; const VULKAN_HPP_NAMESPACE::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & VULKAN_HPP_NAMESPACE::QueueFlagBits::eCompute) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    /**
     * @brief Retrieve the index of compute specialized queue family from \p queueFamilyProperties.
     *
     * <i>Compute specialized</i> means the queue family that supports compute operations but not graphics operations.
     *
     * @param queueFamilyProperties Queue family properties. Could be enumerated by <tt>vk::PhysicalDevice::getQueueFamilyProperties</tt>.
     * @return The index of the compute specialized queue family, or <tt>std::nullopt</tt> if not found.
     */
    export
    [[nodiscard]] std::optional<std::uint32_t> getComputeSpecializedQueueFamily(
        std::span<const VULKAN_HPP_NAMESPACE::QueueFamilyProperties> queueFamilyProperties
    ) noexcept {
        for (std::uint32_t i = 0; const VULKAN_HPP_NAMESPACE::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & VULKAN_HPP_NAMESPACE::QueueFlagBits::eCompute && !(properties.queueFlags & VULKAN_HPP_NAMESPACE::QueueFlagBits::eGraphics)) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    /**
     * @brief Retrieve the index of graphics capable queue family from \p queueFamilyProperties.
     * @param queueFamilyProperties Queue family properties. Could be enumerated by <tt>vk::PhysicalDevice::getQueueFamilyProperties</tt>.
     * @return The index of the graphics capable queue family, or <tt>std::nullopt</tt> if not found.
     */
    export
    [[nodiscard]] std::optional<std::uint32_t> getGraphicsQueueFamily(
        std::span<const VULKAN_HPP_NAMESPACE::QueueFamilyProperties> queueFamilyProperties
    ) noexcept {
        for (std::uint32_t i = 0; const VULKAN_HPP_NAMESPACE::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & VULKAN_HPP_NAMESPACE::QueueFlagBits::eGraphics) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    /**
     * @brief Retrieve the index of transfer specialized queue family from \p queueFamilyProperties.
     *
     * <i>Transfer specialized</i> means the queue family that supports transfer operations but not compute or graphics operations.
     *
     * @param queueFamilyProperties Queue family properties. Could be enumerated by <tt>vk::PhysicalDevice::getQueueFamilyProperties</tt>.
     * @return The index of the transfer specialized queue family, or <tt>std::nullopt</tt> if not found.
     */
    export
    [[nodiscard]] std::optional<std::uint32_t> getTransferSpecializedQueueFamily(
        std::span<const VULKAN_HPP_NAMESPACE::QueueFamilyProperties> queueFamilyProperties
    ) noexcept {
        for (std::uint32_t i = 0; const VULKAN_HPP_NAMESPACE::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & VULKAN_HPP_NAMESPACE::QueueFlagBits::eTransfer && !(properties.queueFlags & (VULKAN_HPP_NAMESPACE::QueueFlagBits::eCompute | VULKAN_HPP_NAMESPACE::QueueFlagBits::eGraphics))) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    /**
     * @brief Retrieve the index of present capable queue family from \p queueFamilyProperties.
     * @param physicalDevice Vulkan physical device to get queue families from.
     * @param surface Surface to test for presentation support.
     * @param queueFamilyCount Number of queue families, which is identical to <tt>physicalDevice.getQueueFamilyProperties().size()</tt>.
     * @return The index of the present capable queue family, or <tt>std::nullopt</tt> if not found.
     */
    export
    [[nodiscard]] std::optional<std::uint32_t> getPresentQueueFamily(
        VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice,
        VULKAN_HPP_NAMESPACE::SurfaceKHR surface,
        std::uint32_t queueFamilyCount
    ) {
        for (std::uint32_t i = 0; i < queueFamilyCount; ++i) {
            if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
                return i;
            }
        }
        return std::nullopt;
    }

    /**
     * @brief Retrieve the index of both graphics and present capable queue family from \p queueFamilyProperties.
     * @param physicalDevice Vulkan physical device to get queue families from.
     * @param surface Surface to test for presentation support.
     * @param queueFamilyProperties Queue family properties, which is identical to <tt>physicalDevice.getQueueFamilyProperties()</tt>.
     * @return The index of the both graphics and present capable queue family, or <tt>std::nullopt</tt> if not found.
     * @note This is useful for the common case of rendering to a window, because it doesn't require the explicit queue
     * family ownership transfer between graphics and present queues.
     */
    export
    [[nodiscard]] std::optional<std::uint32_t> getGraphicsPresentQueueFamily(
        VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice,
        VULKAN_HPP_NAMESPACE::SurfaceKHR surface,
        std::span<const VULKAN_HPP_NAMESPACE::QueueFamilyProperties> queueFamilyProperties
    ) {
        for (std::uint32_t i = 0; const VULKAN_HPP_NAMESPACE::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & VULKAN_HPP_NAMESPACE::QueueFlagBits::eGraphics && physicalDevice.getSurfaceSupportKHR(i, surface)) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    /**
     * @brief Retrieve the index of both compute and present capable queue family from \p queueFamilyProperties.
     * @param physicalDevice Vulkan physical device to get queue families from.
     * @param surface Surface to test for presentation support.
     * @param queueFamilyProperties Queue family properties, which is identical to <tt>physicalDevice.getQueueFamilyProperties()</tt>.
     * @return The index of the both compute and present capable queue family, or <tt>std::nullopt</tt> if not found.
     * @note This is useful for compute based window rendering, because it doesn't require the explicit queue family
     * ownership transfer between compute and present queues.
     */
    export
    [[nodiscard]] std::optional<std::uint32_t> getComputePresentQueueFamily(
        VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice,
        VULKAN_HPP_NAMESPACE::SurfaceKHR surface,
        std::span<const VULKAN_HPP_NAMESPACE::QueueFamilyProperties> queueFamilyProperties
    ) {
        for (std::uint32_t i = 0; const VULKAN_HPP_NAMESPACE::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & VULKAN_HPP_NAMESPACE::QueueFlagBits::eCompute && physicalDevice.getSurfaceSupportKHR(i, surface)) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }
}