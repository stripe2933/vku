/** @file descriptors/PoolSizes.cppm
 */

module;

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:descriptors.PoolSizes;

import std;
import vku.details;
export import vulkan_hpp;
import :utils.RefHolder;

namespace vku {
    export struct PoolSizes {
        std::uint32_t setCount;
        std::unordered_map<VULKAN_HPP_NAMESPACE::DescriptorType, std::uint32_t> typeCounts;

        PoolSizes() noexcept = default;
        PoolSizes(const PoolSizes&) noexcept = default;
        PoolSizes(PoolSizes&&) noexcept = default;

        // Addition/scalar multiplication operators.
        [[nodiscard]] auto operator+(PoolSizes rhs) const noexcept -> PoolSizes;
        auto operator+=(const PoolSizes &rhs) noexcept -> PoolSizes&;
        [[nodiscard]] auto operator*(std::uint32_t multiplier) const noexcept -> PoolSizes;
        auto operator*=(std::uint32_t multiplier) noexcept -> PoolSizes&;

        [[nodiscard]] auto getDescriptorPoolCreateInfo(VULKAN_HPP_NAMESPACE::DescriptorPoolCreateFlags flags = {}) const noexcept -> RefHolder<VULKAN_HPP_NAMESPACE::DescriptorPoolCreateInfo, std::vector<VULKAN_HPP_NAMESPACE::DescriptorPoolSize>>;
    };

    // TODO: looks like this operator overloading is invisible outside the module. Need to investigate.
    export
    [[nodiscard]] auto operator*(std::uint32_t multiplier, PoolSizes rhs) noexcept -> PoolSizes;
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
    VULKAN_HPP_NAMESPACE::DescriptorPoolCreateFlags flags
) const noexcept -> RefHolder<VULKAN_HPP_NAMESPACE::DescriptorPoolCreateInfo, std::vector<VULKAN_HPP_NAMESPACE::DescriptorPoolSize>> {
    std::vector<VULKAN_HPP_NAMESPACE::DescriptorPoolSize> poolSizes;
    for (const auto &[type, count] : typeCounts) {
        poolSizes.emplace_back(type, count);
    }

    return RefHolder {
        [this, flags](std::span<const VULKAN_HPP_NAMESPACE::DescriptorPoolSize> poolSizes) {
            return VULKAN_HPP_NAMESPACE::DescriptorPoolCreateInfo {
                flags,
                setCount,
                poolSizes,
            };
        },
        std::move(poolSizes)
    };
}

auto vku::operator*(std::uint32_t multiplier, PoolSizes rhs) noexcept -> PoolSizes {
    rhs *= multiplier;
    return rhs;
}