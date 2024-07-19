module;

#ifndef VKU_USE_STD_MODULE
#include <compare>
#endif

export module vku:buffers;
export import :buffers.AllocatedBuffer;
export import :buffers.Buffer;
export import :buffers.MappedBuffer;