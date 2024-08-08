module;

#include <cassert>
#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <array>
#include <compare>
#endif

export module vku:descriptors.DescriptorSetLayout;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import vulkan_hpp;
export import :descriptors.PoolSizes;
import :details;

#define INDEX_SEQ(Is, N, ...) [&]<std::size_t ...Is>(std::index_sequence<Is...>) __VA_ARGS__ (std::make_index_sequence<N>{})

namespace vku {
    export template <vk::DescriptorType... BindingTypes>
    struct DescriptorSetLayout : vk::raii::DescriptorSetLayout {
        static constexpr std::uint32_t bindingCount = sizeof...(BindingTypes);
        static constexpr std::array bindingTypes = { BindingTypes... };

        std::array<std::uint32_t, bindingCount> descriptorCounts;

        DescriptorSetLayout(
            const vk::raii::Device &device [[clang::lifetimebound]],
            const vk::DescriptorSetLayoutCreateInfo &createInfo
        ) : vk::raii::DescriptorSetLayout { device, createInfo } {
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

    export template <concepts::derived_from_value_specialization_of<DescriptorSetLayout>... Layouts>
    [[nodiscard]] auto getPoolSizes(const Layouts &...layouts) noexcept -> PoolSizes {
        return (layouts.getPoolSize() + ...);
    }
}