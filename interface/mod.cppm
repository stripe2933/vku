module;

#if !defined(VKU_USE_STD_MODULE) && defined(_MSC_VER)
#include <compare>
#endif

export module vku;
export import :buffers;
export import :constants;
export import :debugging;
export import :descriptors;
export import :Gpu;
export import :commands;
export import :images;
export import :pipelines;
export import :queue;
export import :rendering;
export import :utils;