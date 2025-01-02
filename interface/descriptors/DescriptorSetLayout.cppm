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
    /**
     * @brief Owning descriptor set layout with compile-time binding types.
     *
     * @code{.cpp}
     * const vk::raii::Device device { ... };
     *
     * // Create descriptor set layout that would be used for a descriptor set, whose 0-th binding is pointing a uniform buffer and 1-th binding is pointing to multiple combined image samplers.
     * const vku::DescriptorSetLayout<vk::DescriptorType::eUniformBuffer, vk::DescriptorType::eCombinedImageSampler> descriptorSetLayout {
     *     device,
     *     vk::DescriptorSetLayoutCreateInfo {
     *         {},
     *         vku::unsafeProxy({
     *             vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex },
     *             vk::DescriptorSetLayoutBinding { 1, vk::DescriptorType::eCombinedImageSampler, 3, vk::ShaderStageFlagBits::eFragment },
     *         }),
     *     },
     * };
     *
     * static_assert(descriptorSetLayout.bindingCount == 2);
     * static_assert(descriptorSetLayout.bindingTypes == std::array { vk::DescriptorType::eUniformBuffer, vk::DescriptorType::eCombinedImageSampler });
     * assert(descriptorSetLayout.descriptorCounts == std::array { 1, 3 });
     * @endcode
     *
     * @tparam BindingTypes Variadic NTTP of descriptor types.
     */
    export template <VULKAN_HPP_NAMESPACE::DescriptorType... BindingTypes>
    struct DescriptorSetLayout : VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::DescriptorSetLayout {
        template <VULKAN_HPP_NAMESPACE::DescriptorType BindingType>
        struct BindingInfo {
            std::uint32_t descriptorCount;
            VULKAN_HPP_NAMESPACE::ShaderStageFlags stageFlags;
            const VULKAN_HPP_NAMESPACE::Sampler *pImmutableSamplers;
        };

        /**
         * @brief Number of bindings.
         */
        static constexpr std::uint32_t bindingCount = sizeof...(BindingTypes);

        /**
         * @brief Array of binding types. <tt>bindingTypes[I]</tt> is the type of the <tt>I</tt>-th binding.
         */
        static constexpr std::array bindingTypes = { BindingTypes... };

        /**
         * @brief Number of descriptors for each binding. <tt>descriptorCounts[I]</tt> is the number of descriptors for the <tt>I</tt>-th binding.
         */
        std::array<std::uint32_t, bindingCount> descriptorCounts;

        /**
         * @brief Create a RAII descriptor set layout with the specified \p device and \p createInfo.
         * @param device Vulkan device for RAII descriptor set layout object creation.
         * @param createInfo Descriptor set layout create info. Its bindings must match the template parameter count and types.
         */
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

        /**
         * @brief Create binding array with only descriptor count and stage (and immutable samplers if specified). Binding indices and types are used from the template parameters.
         *
         * @code{.cpp}
         * struct MyDescriptorSetLayout : vku::DescriptorSetLayout<vk::DescriptorType::eUniformBuffer, vk::DescriptorType::eCombinedImageSampler> {
         *     explicit MyDescriptorSetLayout(const vk::raii::Device &device)
         *         : DescriptorSetLayout { device, vk::DescriptorSetLayoutCreateInfo {
         *             {},
         *             vku::unsafeProxy(getBindings({ 1, vk::ShaderStageFlagBits::eVertex }, { 3, vk::ShaderStageFlagBits::eFragment })),
         *         } { }
         * };
         * @endcode
         *
         * @param bindingInfos Binding infos for each binding. These will be applied with index starts from zero.
         * @return Array of vk::DescriptorSetLayoutBinding with the specified binding infos.
         */
        [[nodiscard]] static constexpr std::array<VULKAN_HPP_NAMESPACE::DescriptorSetLayoutBinding, bindingCount> getBindings(
            const BindingInfo<BindingTypes> &...bindingInfos
        ) noexcept {
            return INDEX_SEQ(Is, bindingCount, {
                return std::array { VULKAN_HPP_NAMESPACE::DescriptorSetLayoutBinding {
                    Is,
                    BindingTypes,
                    bindingInfos.descriptorCount,
                    bindingInfos.stageFlags,
                    bindingInfos.pImmutableSamplers,
                }... };
            });
        }

        /**
         * @brief PoolSizes that allocating a single descriptor set with this layout requires.
         *
         * Allocating a single descriptor set with this layout will require descriptors of the specified types and a single set.
         *
         * @return Required pool sizes.
         */
        [[nodiscard]] PoolSizes getPoolSize() const noexcept {
            PoolSizes poolSizes;
            poolSizes.setCount = 1;
            apply([&](auto... counts) {
                ((poolSizes.typeCounts[BindingTypes] += counts), ...);
            }, descriptorCounts);
            return poolSizes;
        }
    };

    /**
     * @brief Get the required pool sizes for the specified descriptor set layouts.
     * @tparam Layouts Variadic NTTP of descriptor set layouts.
     * @param layouts Descriptor set layouts to get the pool sizes.
     * @return Required pool sizes.
     */
    export template <details::derived_from_value_specialization_of<DescriptorSetLayout>... Layouts>
    [[nodiscard]] PoolSizes getPoolSizes(const Layouts &...layouts) noexcept {
        return (layouts.getPoolSize() + ...);
    }
}