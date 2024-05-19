#pragma once

#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "DescriptorSetLayouts.hpp"
#include "details/ranges.hpp"

namespace vku {
    template <details::derived_from_value_specialization_of<DescriptorSetLayouts> Layouts>
    class DescriptorSets : public std::array<vk::DescriptorSet, Layouts::setCount> {
    public:
        DescriptorSets(
            vk::Device device,
            vk::DescriptorPool descriptorPool,
            const Layouts &layouts
        ) : std::array<vk::DescriptorSet, Layouts::setCount> { device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo {
                descriptorPool,
                layouts,
            }) | ::details::ranges::to_array<Layouts::setCount>() },
            layouts { layouts } { }

        // For push descriptor usage.
        explicit DescriptorSets(
            const Layouts &layouts
        ) : layouts { layouts } { }

        template <typename Self>
        [[nodiscard]] static auto allocateMultiple(
            vk::Device device,
            vk::DescriptorPool descriptorPool,
            const Layouts &descriptorSetLayouts,
            std::size_t n
        ) -> std::vector<Self> {
            const std::vector multipleSetLayouts
                // TODO: 1. Why pipe syntax between std::span and std::views::repeat not works? 2. Why compile-time sized span not works?
                = std::views::repeat(std::span<const vk::DescriptorSetLayout> { descriptorSetLayouts }, n)
                | std::views::join
                | std::ranges::to<std::vector>();

            const std::vector descriptorSets = device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo {
                descriptorPool,
                multipleSetLayouts,
            });
            // TODO.CXX23: use std::views::chunk instead, like:
            // return descriptorSets
            //     | std::views::chunk(Layouts::setCount)
            //     | std::views::transform([&](const auto &sets) {
            //         return Self { descriptorSetLayouts, sets | ::details::ranges::to_array<Layouts::setCount>() };
            //     })
            //     | std::ranges::to<std::vector>();
            return std::views::iota(std::size_t { 0 }, n)
                | std::views::transform([&](std::size_t i) {
                    return Self {
                        descriptorSetLayouts,
                        std::views::counted(descriptorSets.data() + i * Layouts::setCount, Layouts::setCount)
                            | ::details::ranges::to_array<Layouts::setCount>(),
                    };
                })
                | std::ranges::to<std::vector>();
        }

    protected:
        template <std::uint32_t Set, std::uint32_t Binding>
        [[nodiscard]] auto getDescriptorWrite() const -> vk::WriteDescriptorSet {
            return {
                get<Set>(*this), // Error in here: you specify set index that exceeds the number of descriptor set layouts.
                Binding,
                0,
                {},
                get<Binding>(get<Set>(layouts.setLayouts)).descriptorType, // Error in here: you specify binding index that exceeds the number of layout bindings in the set.
            };
        }

    private:
        const Layouts &layouts;

        // For allocateMultiple.
        DescriptorSets(
            const Layouts &layouts,
            const std::array<vk::DescriptorSet, Layouts::setCount> &descriptorSets
        ) : std::array<vk::DescriptorSet, Layouts::setCount> { descriptorSets },
            layouts { layouts } { }
    };
}