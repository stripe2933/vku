#pragma once

#ifdef VKU_USE_GLM
#include <glm/glm.hpp>
#endif
#include <vulkan/vulkan.hpp>

namespace vku{
#ifdef VKU_USE_GLM
    [[nodiscard]] constexpr auto convertExtent2D(
        const glm::uvec2 &v
    ) noexcept -> vk::Extent2D {
        return { v.x, v.y };
    }
#endif

    [[nodiscard]] constexpr auto convertExtent2D(
        const vk::Extent3D &extent
    ) noexcept -> vk::Extent2D {
        return { extent.width, extent.height };
    }

    [[nodiscard]] constexpr auto convertOffset2D(
        const vk::Extent2D &extent
    ) noexcept -> vk::Offset2D {
        return { static_cast<std::int32_t>(extent.width), static_cast<std::int32_t>(extent.height) };
    }

    [[nodiscard]] constexpr auto convertOffset3D(
        const vk::Extent3D &extent
    ) noexcept -> vk::Offset3D {
        return { static_cast<std::int32_t>(extent.width), static_cast<std::int32_t>(extent.height), static_cast<std::int32_t>(extent.depth) };
    }

    /**
     * @brief Get aspect ratio of \p vk::Extent2D rectangle.
     * @param extent
     * @return Aspect ratio, i.e. width / height.
     */
    [[nodiscard]] constexpr auto aspect(
        const vk::Extent2D &extent
    ) noexcept -> float {
        return static_cast<float>(extent.width) / extent.height;
    }

    [[nodiscard]] constexpr auto toFlags(auto flagBit) noexcept -> vk::Flags<decltype(flagBit)> {
        return flagBit;
    }

    template <typename T>
    [[nodiscard]] constexpr auto contains(vk::Flags<T> flags, T flag) noexcept -> bool {
        return (flags & flag) == flag;
    }

    template <typename T>
    [[nodiscard]] constexpr auto contains(vk::Flags<T> flags, vk::Flags<T> flag) noexcept -> bool {
        return (flags & flag) == flag;
    }

    template <std::unsigned_integral T>
    [[nodiscard]] constexpr auto divCeil(T num, T denom) noexcept -> T {
        return (num / denom) + (num % denom != 0);
    }

    [[nodiscard]] constexpr auto workgroupTotal(
        std::span<const std::uint32_t, 3> workgroupCount
    ) noexcept -> std::uint32_t {
        return workgroupCount[0] * workgroupCount[1] * workgroupCount[2];
    }
}