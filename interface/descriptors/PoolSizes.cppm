module;

#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <compare>
#include <ranges>
#include <span>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>
#endif

export module vku:descriptors.PoolSizes;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vulkan_hpp;
import :details;
import :utils.RefHolder;

namespace vku {
    export struct PoolSizes {
        std::uint32_t setCount;
        std::unordered_map<vk::DescriptorType, std::uint32_t> typeCounts;

        PoolSizes() noexcept = default;
        PoolSizes(const PoolSizes&) noexcept = default;
        PoolSizes(PoolSizes&&) noexcept = default;

        // Addition/scalar multiplication operators.
        [[nodiscard]] auto operator+(PoolSizes rhs) const noexcept -> PoolSizes;
        auto operator+=(const PoolSizes &rhs) noexcept -> PoolSizes&;
        [[nodiscard]] auto operator*(std::uint32_t multiplier) const noexcept -> PoolSizes;
        friend auto operator*(std::uint32_t multiplier, const PoolSizes &rhs) noexcept -> PoolSizes;
        auto operator*=(std::uint32_t multiplier) noexcept -> PoolSizes&;

        [[nodiscard]] auto getDescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlags flags = {}) const noexcept -> RefHolder<vk::DescriptorPoolCreateInfo, std::vector<vk::DescriptorPoolSize>>;
    };
}

[[nodiscard]] auto operator*(std::uint32_t multiplier, const vku::PoolSizes &rhs) noexcept -> vku::PoolSizes {
    return rhs * multiplier;
}

// module :private;

auto vku::PoolSizes::operator+(PoolSizes rhs) const noexcept -> PoolSizes {
    rhs.setCount += setCount;
    for (const auto &[type, count] : typeCounts) {
        rhs.typeCounts[type] += count;
    }
    return rhs;
}

auto vku::PoolSizes::operator+=(const PoolSizes &rhs) noexcept -> PoolSizes & {
    setCount += rhs.setCount;
    for (const auto &[type, count] : rhs.typeCounts) {
        typeCounts[type] += count;
    }
    return *this;
}

auto vku::PoolSizes::operator*(std::uint32_t multiplier) const noexcept -> PoolSizes {
    PoolSizes result { *this };
    result.setCount *= multiplier;
    for (std::uint32_t &count : result.typeCounts | std::views::values) {
        count *= multiplier;
    }
    return result;
}

auto vku::PoolSizes::operator*=(std::uint32_t multiplier) noexcept -> PoolSizes & {
    setCount *= multiplier;
    for (std::uint32_t &count : typeCounts | std::views::values) {
        count *= multiplier;
    }
    return *this;
}

auto vku::PoolSizes::getDescriptorPoolCreateInfo(
    vk::DescriptorPoolCreateFlags flags
) const noexcept -> RefHolder<vk::DescriptorPoolCreateInfo, std::vector<vk::DescriptorPoolSize>> {
    return RefHolder {
        [=, this](std::span<const vk::DescriptorPoolSize> poolSizes) {
            return vk::DescriptorPoolCreateInfo {
                flags,
                setCount,
                poolSizes,
            };
        },
        typeCounts
            | std::views::transform([](const auto &keyValue) {
                return vk::DescriptorPoolSize { keyValue.first, keyValue.second };
            })
            | std::ranges::to<std::vector>(),
    };
}
