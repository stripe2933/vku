module;

#ifndef VKU_USE_STD_MODULE
#include <cstdint>
#include <compare>
#include <tuple>
#include <utility>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

export module vku:descriptors.DescriptorSet;

#ifdef VKU_USE_STD_MODULE
import std;
#endif
export import :descriptors.DescriptorSetLayout;
import :details;
import :utils;

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
    export template <concepts::derived_from_value_specialization_of<DescriptorSetLayout> Layout>
    class DescriptorSet : public VULKAN_HPP_NAMESPACE::DescriptorSet {
    public:
        DescriptorSet() noexcept = default;
        DescriptorSet(const DescriptorSet&) noexcept = default;
        DescriptorSet(DescriptorSet&&) noexcept = default;
        DescriptorSet& operator=(const DescriptorSet&) noexcept = default;
        DescriptorSet& operator=(DescriptorSet&&) noexcept = default;

        template <std::uint32_t Binding>
        [[nodiscard]] auto getWrite(const VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const WriteDescriptorInfo_t<get<Binding>(Layout::bindingTypes)>> &descriptorInfos) const noexcept -> VULKAN_HPP_NAMESPACE::WriteDescriptorSet {
            constexpr auto attachInfo = multilambda {
                [](VULKAN_HPP_NAMESPACE::WriteDescriptorSet writeDescriptorSet, const VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const VULKAN_HPP_NAMESPACE::DescriptorImageInfo> &descriptorInfos) {
#ifdef VULKAN_HPP_NO_SETTERS
                    writeDescriptorSet.descriptorCount = static_cast<std::uint32_t>(descriptorInfos.size());
                    writeDescriptorSet.pImageInfo = descriptorInfos.data();
                    return writeDescriptorSet;
#else
                    return writeDescriptorSet.setImageInfo(descriptorInfos);
#endif
                },
                [](VULKAN_HPP_NAMESPACE::WriteDescriptorSet writeDescriptorSet, const VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const VULKAN_HPP_NAMESPACE::BufferView> &descriptorInfos) {
#ifdef VULKAN_HPP_NO_SETTERS
                    writeDescriptorSet.descriptorCount = static_cast<std::uint32_t>(descriptorInfos.size());
                    writeDescriptorSet.pTexelBufferView = descriptorInfos.data();
                    return writeDescriptorSet;
#else
                    return writeDescriptorSet.setTexelBufferView(descriptorInfos);
#endif
                },
                [](VULKAN_HPP_NAMESPACE::WriteDescriptorSet writeDescriptorSet, const VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<const VULKAN_HPP_NAMESPACE::DescriptorBufferInfo> &descriptorInfos) {
#ifdef VULKAN_HPP_NO_SETTERS
                    writeDescriptorSet.descriptorCount = static_cast<std::uint32_t>(descriptorInfos.size());
                    writeDescriptorSet.pBufferInfo = descriptorInfos.data();
                    return writeDescriptorSet;
#else
                    return writeDescriptorSet.setBufferInfo(descriptorInfos);
#endif
                },
            };

            return attachInfo(VULKAN_HPP_NAMESPACE::WriteDescriptorSet { *this, Binding, 0, {}, get<Binding>(Layout::bindingTypes) }, descriptorInfos);
        }

        /**
         * Convenience method for calling <tt>getWrite<Binding></tt> with single lifetime-bounded descriptor info object.
         * @tparam Binding Binding index to get the write descriptor.
         * @param descriptorInfo Descriptor info to write. This is either <tt>vk::DescriptorBufferInfo</tt>,
         * <tt>vk::DescriptorImageInfo</tt> or <tt>vk::BufferView</tt>, based on your descriptor type predefined by <tt>DescriptorSetLayout</tt>.
         * @return <tt>vk::WriteDescriptorSet</tt> with given info.
         * @example
         * @code
         * struct Layout : vku::DescriptorSetLayout<vk::DescriptorType::eStorageBuffer> { ... } layout;
         *
         * auto [descriptorSet] = vku::allocateDescriptorSets(*device, *descriptorPool, std::tie(layout));
         * device.updateDescriptorSets({
         *     descriptorSet.getWriteOne<0>({ buffer, 0, vk::WholeSize }), // argument type infered as vk::DescriptorBufferInfo at the compile time.
         * }, {});
         * @endcode
         */
        template <std::uint32_t Binding>
        [[nodiscard]] auto getWriteOne(const WriteDescriptorInfo_t<get<Binding>(Layout::bindingTypes)> &descriptorInfo [[clang::lifetimebound]]) const noexcept -> VULKAN_HPP_NAMESPACE::WriteDescriptorSet {
            return getWrite<Binding>(descriptorInfo);
        }

        template <concepts::derived_from_value_specialization_of<DescriptorSetLayout>... Layouts>
        friend auto allocateDescriptorSets(VULKAN_HPP_NAMESPACE::Device, VULKAN_HPP_NAMESPACE::DescriptorPool, std::tuple<const Layouts&...>) -> std::tuple<DescriptorSet<Layouts>...>;

    private:
        explicit DescriptorSet(VULKAN_HPP_NAMESPACE::DescriptorSet descriptorSet) noexcept : VULKAN_HPP_NAMESPACE::DescriptorSet { descriptorSet } {}
    };

    export template <concepts::derived_from_value_specialization_of<DescriptorSetLayout>... Layouts>
    [[nodiscard]] auto allocateDescriptorSets(
        VULKAN_HPP_NAMESPACE::Device device,
        VULKAN_HPP_NAMESPACE::DescriptorPool pool,
        std::tuple<const Layouts&...> layouts
    ) -> std::tuple<DescriptorSet<Layouts>...> {
        return std::apply([&](const auto &...layout) {
            return std::apply([&](auto... rawSet) {
                return std::tuple { DescriptorSet<Layouts> { rawSet }... };
            }, device.allocateDescriptorSets({ pool, unsafeProxy({ *layout... }) }) | ranges::to_array<sizeof...(Layouts)>());
        }, layouts);
    }
}