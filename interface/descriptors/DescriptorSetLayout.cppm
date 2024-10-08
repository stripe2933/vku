/** @file descriptors/DescriptorSetLayout.cppm
 */

module;

#include <cassert>
#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <array>
#ifdef _MSC_VER
#include <compare>
#endif
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:descriptors.DescriptorSetLayout;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vulkan_hpp;
export import :descriptors.PoolSizes;
import :details.concepts;

#define INDEX_SEQ(Is, N, ...) [&]<std::size_t ...Is>(std::index_sequence<Is...>) __VA_ARGS__ (std::make_index_sequence<N>{})

namespace vku {
    export template <VULKAN_HPP_NAMESPACE::DescriptorType... BindingTypes>
    struct DescriptorSetLayout : VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::DescriptorSetLayout {
        static constexpr std::uint32_t bindingCount = sizeof...(BindingTypes);
        static constexpr std::array bindingTypes = { BindingTypes... };

        std::array<std::uint32_t, bindingCount> descriptorCounts;

        DescriptorSetLayout(
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device [[clang::lifetimebound]],
            const VULKAN_HPP_NAMESPACE::DescriptorSetLayoutCreateInfo &createInfo
        ) : VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::DescriptorSetLayout { device, createInfo } {
            assert(createInfo.bindingCount == bindingCount && "The binding count must match the template parameter count.");
            INDEX_SEQ(Is, bindingCount, {
                assert(((createInfo.pBindings[Is].descriptorType == BindingTypes) && ...) && "The descriptor types must match the template parameter.");
                ((descriptorCounts[Is] = createInfo.pBindings[Is].descriptorCount), ...);
            });
        }

        [[nodiscard]] auto getPoolSize() const noexcept -> PoolSizes {
            PoolSizes poolSizes;
            poolSizes.setCount = 1;
            apply([&](auto... counts) {
                ((poolSizes.typeCounts[BindingTypes] += counts), ...);
            }, descriptorCounts);
            return poolSizes;
        }
    };

    export template <details::derived_from_value_specialization_of<DescriptorSetLayout>... Layouts>
    [[nodiscard]] auto getPoolSizes(const Layouts &...layouts) noexcept -> PoolSizes {
        return (layouts.getPoolSize() + ...);
    }
}