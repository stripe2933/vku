/** @file descriptors/DescriptorSet.cppm
 */

module;

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:descriptors.DescriptorSet;

import std;
export import :descriptors.DescriptorSetLayout;
import :details.concepts;
import :utils;

#define INDEX_SEQ(Is, N, ...) [&]<std::size_t ...Is>(std::index_sequence<Is...>) __VA_ARGS__ (std::make_index_sequence<N>{})

namespace vku {
    export template <details::derived_from_value_specialization_of<DescriptorSetLayout> Layout>
    struct DescriptorSet : VULKAN_HPP_NAMESPACE::DescriptorSet {
        DescriptorSet() noexcept = default;
        explicit DescriptorSet(unsafe_t, VULKAN_HPP_NAMESPACE::DescriptorSet descriptorSet) noexcept
            : VULKAN_HPP_NAMESPACE::DescriptorSet { descriptorSet } {}
        DescriptorSet(const DescriptorSet&) noexcept = default;
        DescriptorSet(DescriptorSet&&) noexcept = default;
        DescriptorSet& operator=(const DescriptorSet&) noexcept = default;
        DescriptorSet& operator=(DescriptorSet&&) noexcept = default;

        template <std::uint32_t Binding>
        [[nodiscard]] VULKAN_HPP_NAMESPACE::WriteDescriptorSet getWrite(
            const VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const WriteDescriptorInfo_t<get<Binding>(Layout::bindingTypes)>> &descriptorInfos
        ) const noexcept {
            VULKAN_HPP_NAMESPACE::WriteDescriptorSet writeDescriptorSet = Layout::template getWrite<Binding>(descriptorInfos);
            writeDescriptorSet.dstSet = *this;
            return writeDescriptorSet;
        }

        /**
         * Convenience method for calling <tt>getWrite<Binding></tt> with single lifetime-bounded descriptor info object.
         * @code{.cpp}
         * struct Layout : vku::DescriptorSetLayout<vk::DescriptorType::eStorageBuffer> { ... } layout;
         *
         * auto [descriptorSet] = vku::allocateDescriptorSets(*device, *descriptorPool, std::tie(layout));
         * device.updateDescriptorSets({
         *     descriptorSet.getWriteOne<0>({ buffer, 0, vk::WholeSize }), // argument type infered as vk::DescriptorBufferInfo at the compile time.
         * }, {});
         * @endcode
         * @tparam Binding Binding index to get the write descriptor.
         * @param descriptorInfo Descriptor info to write. This is either <tt>vk::DescriptorBufferInfo</tt>, <tt>vk::DescriptorImageInfo</tt> or <tt>vk::BufferView</tt>, based on your descriptor type predefined by <tt>DescriptorSetLayout</tt>.
         * @return <tt>vk::WriteDescriptorSet</tt> with given info.
         */
        template <std::uint32_t Binding>
        [[nodiscard]] VULKAN_HPP_NAMESPACE::WriteDescriptorSet getWriteOne(
            const WriteDescriptorInfo_t<get<Binding>(Layout::bindingTypes)> &&descriptorInfo [[clang::lifetimebound]]
        ) const noexcept {
            return getWrite<Binding>(descriptorInfo);
        }

        template <details::derived_from_value_specialization_of<DescriptorSetLayout>... Layouts>
        friend auto allocateDescriptorSets(VULKAN_HPP_NAMESPACE::Device, VULKAN_HPP_NAMESPACE::DescriptorPool, const std::tuple<Layouts...> &layouts) -> std::tuple<DescriptorSet<std::remove_cvref_t<Layouts>>...>;
    };

    export template <details::derived_from_value_specialization_of<DescriptorSetLayout>... Layouts>
    [[nodiscard]] auto allocateDescriptorSets(
        VULKAN_HPP_NAMESPACE::Device device,
        VULKAN_HPP_NAMESPACE::DescriptorPool pool,
        const std::tuple<Layouts...> &layouts
    ) -> std::tuple<DescriptorSet<std::remove_cvref_t<Layouts>>...> {
        const std::array rawDescriptorSetLayouts = std::apply([&](const auto &...layout) {
            return std::array { *layout... };
        }, layouts);
        const std::vector rawDescriptorSets = device.allocateDescriptorSets({ pool, rawDescriptorSetLayouts });
        return INDEX_SEQ(Is, sizeof...(Layouts), {
            return std::tuple { DescriptorSet<std::remove_cvref_t<Layouts>> { unsafe, rawDescriptorSets[Is] }... };
        });
    }
}