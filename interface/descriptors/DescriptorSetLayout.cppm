/** @file descriptors/DescriptorSetLayout.cppm
 */

module;

#include <cassert>

#include <vulkan/vulkan_hpp_macros.hpp>

#include <lifetimebound.hpp>

export module vku:descriptors.DescriptorSetLayout;

import std;
import vku.details;
export import vulkan_hpp;
export import :descriptors.PoolSizes;

#define INDEX_SEQ(Is, N, ...) [&]<std::size_t ...Is>(std::index_sequence<Is...>) __VA_ARGS__ (std::make_index_sequence<N>{})

template <VULKAN_HPP_NAMESPACE::DescriptorType> struct WriteDescriptorInfo;
template <> struct WriteDescriptorInfo<VULKAN_HPP_NAMESPACE::DescriptorType::eSampler> { using type = VULKAN_HPP_NAMESPACE::DescriptorImageInfo; };
template <> struct WriteDescriptorInfo<VULKAN_HPP_NAMESPACE::DescriptorType::eCombinedImageSampler> { using type = VULKAN_HPP_NAMESPACE::DescriptorImageInfo; };
template <> struct WriteDescriptorInfo<VULKAN_HPP_NAMESPACE::DescriptorType::eSampledImage> { using type = VULKAN_HPP_NAMESPACE::DescriptorImageInfo; };
template <> struct WriteDescriptorInfo<VULKAN_HPP_NAMESPACE::DescriptorType::eStorageImage> { using type = VULKAN_HPP_NAMESPACE::DescriptorImageInfo; };
template <> struct WriteDescriptorInfo<VULKAN_HPP_NAMESPACE::DescriptorType::eUniformTexelBuffer> { using type = VULKAN_HPP_NAMESPACE::BufferView; };
template <> struct WriteDescriptorInfo<VULKAN_HPP_NAMESPACE::DescriptorType::eStorageTexelBuffer> { using type = VULKAN_HPP_NAMESPACE::BufferView; };
template <> struct WriteDescriptorInfo<VULKAN_HPP_NAMESPACE::DescriptorType::eUniformBuffer> { using type = VULKAN_HPP_NAMESPACE::DescriptorBufferInfo; };
template <> struct WriteDescriptorInfo<VULKAN_HPP_NAMESPACE::DescriptorType::eStorageBuffer> { using type = VULKAN_HPP_NAMESPACE::DescriptorBufferInfo; };
template <> struct WriteDescriptorInfo<VULKAN_HPP_NAMESPACE::DescriptorType::eUniformBufferDynamic> { using type = VULKAN_HPP_NAMESPACE::DescriptorBufferInfo; };
template <> struct WriteDescriptorInfo<VULKAN_HPP_NAMESPACE::DescriptorType::eStorageBufferDynamic> { using type = VULKAN_HPP_NAMESPACE::DescriptorBufferInfo; };
template <> struct WriteDescriptorInfo<VULKAN_HPP_NAMESPACE::DescriptorType::eInputAttachment> { using type = VULKAN_HPP_NAMESPACE::DescriptorImageInfo; };
template <VULKAN_HPP_NAMESPACE::DescriptorType Type> using WriteDescriptorInfo_t = typename WriteDescriptorInfo<Type>::type;

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
            const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device LIFETIMEBOUND,
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

        template <std::uint32_t Binding>
        [[nodiscard]] static VULKAN_HPP_NAMESPACE::WriteDescriptorSet getWrite(
            const VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const WriteDescriptorInfo_t<get<Binding>(bindingTypes)>> &descriptorInfos
        ) noexcept {
            VULKAN_HPP_NAMESPACE::WriteDescriptorSet writeDescriptorSet { nullptr, Binding, 0, {}, get<Binding>(bindingTypes) };
            writeDescriptorSet.descriptorCount = static_cast<std::uint32_t>(descriptorInfos.size());
            details::multilambda {
                [&](const VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const VULKAN_HPP_NAMESPACE::DescriptorImageInfo> &descriptorInfos) {
                    writeDescriptorSet.pImageInfo = descriptorInfos.data();
                },
                [&](const VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const VULKAN_HPP_NAMESPACE::BufferView> &descriptorInfos) {
                    writeDescriptorSet.pTexelBufferView = descriptorInfos.data();
                },
                [&](const VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const VULKAN_HPP_NAMESPACE::DescriptorBufferInfo> &descriptorInfos) {
                    writeDescriptorSet.pBufferInfo = descriptorInfos.data();
                },
            }(descriptorInfos);
            return writeDescriptorSet;
        }

        /**
         * Convenience method for calling <tt>getWrite<Binding></tt> with single lifetime-bounded descriptor info object.
         * @code{.cpp}
         * struct Layout : vku::DescriptorSetLayout<vk::DescriptorType::eStorageBuffer> { ... };
         *
         * cb.pushDescriptorSetKHR(
         *     vk::PipelineBindPoint::eCompute, *pipelineLayout,
         *     0, Layout::getWriteOne<0>({ buffer, 0, vk::WholeSize })); // argument type infered as vk::DescriptorBufferInfo at the compile time.
         * @endcode
         * @tparam Binding Binding index to get the write descriptor.
         * @param descriptorInfo Descriptor info to write. This is either <tt>vk::DescriptorBufferInfo</tt>, <tt>vk::DescriptorImageInfo</tt> or <tt>vk::BufferView</tt>, based on your descriptor type predefined by <tt>DescriptorSetLayout</tt>.
         * @return <tt>vk::WriteDescriptorSet</tt> with given info.
         */
        template <std::uint32_t Binding>
        [[nodiscard]] static VULKAN_HPP_NAMESPACE::WriteDescriptorSet getWriteOne(
            const WriteDescriptorInfo_t<get<Binding>(bindingTypes)> &&descriptorInfo LIFETIMEBOUND
        ) noexcept {
            return getWrite<Binding>(descriptorInfo);
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