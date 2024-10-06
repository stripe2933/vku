module;

#ifndef VKU_USE_STD_MODULE
#include <concepts>
#include <ranges>
#endif

export module vku:details.ranges;

#ifdef VKU_USE_STD_MODULE
import std;
#endif

namespace vku::ranges {
    export template <typename R, typename T>
    concept contiguous_range_of = std::ranges::contiguous_range<R> && std::same_as<std::ranges::range_value_t<R>, T>;
}