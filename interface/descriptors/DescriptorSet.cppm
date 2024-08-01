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

template <vk::DescriptorType> struct WriteDescriptorInfo;
template <> struct WriteDescriptorInfo<vk::DescriptorType::eSampler> { using type = vk::DescriptorImageInfo; };
template <> struct WriteDescriptorInfo<vk::DescriptorType::eCombinedImageSampler> { using type = vk::DescriptorImageInfo; };
template <> struct WriteDescriptorInfo<vk::DescriptorType::eSampledImage> { using type = vk::DescriptorImageInfo; };
template <> struct WriteDescriptorInfo<vk::DescriptorType::eStorageImage> { using type = vk::DescriptorImageInfo; };
template <> struct WriteDescriptorInfo<vk::DescriptorType::eUniformTexelBuffer> { using type = vk::BufferView; };
template <> struct WriteDescriptorInfo<vk::DescriptorType::eStorageTexelBuffer> { using type = vk::BufferView; };
template <> struct WriteDescriptorInfo<vk::DescriptorType::eUniformBuffer> { using type = vk::DescriptorBufferInfo; };
template <> struct WriteDescriptorInfo<vk::DescriptorType::eStorageBuffer> { using type = vk::DescriptorBufferInfo; };
template <> struct WriteDescriptorInfo<vk::DescriptorType::eUniformBufferDynamic> { using type = vk::DescriptorBufferInfo; };
template <> struct WriteDescriptorInfo<vk::DescriptorType::eStorageBufferDynamic> { using type = vk::DescriptorBufferInfo; };
template <> struct WriteDescriptorInfo<vk::DescriptorType::eInputAttachment> { using type = vk::DescriptorImageInfo; };
template <vk::DescriptorType Type> using WriteDescriptorInfo_t = typename WriteDescriptorInfo<Type>::type;

namespace vku {
    export template <concepts::derived_from_value_specialization_of<DescriptorSetLayout> Layout>
    class DescriptorSet : public vk::DescriptorSet {
    public:
        DescriptorSet() noexcept = default;
        DescriptorSet(const DescriptorSet&) noexcept = default;
        DescriptorSet(DescriptorSet&&) noexcept = default;
        DescriptorSet& operator=(const DescriptorSet&) noexcept = default;
        DescriptorSet& operator=(DescriptorSet&&) noexcept = default;

        template <std::uint32_t Binding>
        [[nodiscard]] auto getWrite(const vk::ArrayProxyNoTemporaries<const WriteDescriptorInfo_t<get<Binding>(Layout::bindingTypes)>> &descriptorInfos) const noexcept -> vk::WriteDescriptorSet {
            constexpr auto attachInfo = multilambda {
                [](vk::WriteDescriptorSet writeDescriptorSet, const vk::ArrayProxyNoTemporaries<const vk::DescriptorImageInfo> &descriptorInfos) {
                    return writeDescriptorSet.setImageInfo(descriptorInfos);
                },
                [](vk::WriteDescriptorSet writeDescriptorSet, const vk::ArrayProxyNoTemporaries<const vk::BufferView> &descriptorInfos) {
                    return writeDescriptorSet.setTexelBufferView(descriptorInfos);
                },
                [](vk::WriteDescriptorSet writeDescriptorSet, const vk::ArrayProxyNoTemporaries<const vk::DescriptorBufferInfo> &descriptorInfos) {
                    return writeDescriptorSet.setBufferInfo(descriptorInfos);
                },
            };

            return attachInfo(vk::WriteDescriptorSet { *this, Binding, 0, {}, get<Binding>(Layout::bindingTypes) }, descriptorInfos);
        }

        template <concepts::derived_from_value_specialization_of<DescriptorSetLayout>... Layouts>
        friend auto allocateDescriptorSets(vk::Device, vk::DescriptorPool, std::tuple<const Layouts&...>) -> std::tuple<DescriptorSet<Layouts>...>;

    private:
        explicit DescriptorSet(vk::DescriptorSet descriptorSet) noexcept : vk::DescriptorSet { descriptorSet } {}
    };

    export template <concepts::derived_from_value_specialization_of<DescriptorSetLayout>... Layouts>
    [[nodiscard]] auto allocateDescriptorSets(
        vk::Device device,
        vk::DescriptorPool pool,
        std::tuple<const Layouts&...> layouts
    ) -> std::tuple<DescriptorSet<Layouts>...> {
        return std::apply([&](const auto &...layout) {
            return std::apply([&](auto... rawSet) {
                return std::tuple { DescriptorSet<Layouts> { rawSet }... };
            }, device.allocateDescriptorSets({ pool, unsafeProxy({ *layout... }) }) | ranges::to_array<sizeof...(Layouts)>());
        }, layouts);
    }
}