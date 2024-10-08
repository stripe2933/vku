/** @file descriptors/mod.cppm
 */

module;

#if !defined(VKU_USE_STD_MODULE) && defined(_MSC_VER)
#include <compare>
#endif

export module vku:descriptors;
export import :descriptors.DescriptorSetLayout;
export import :descriptors.DescriptorSet;
export import :descriptors.PoolSizes;