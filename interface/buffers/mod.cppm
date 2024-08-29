module;

#if !defined(VKU_USE_STD_MODULE) && defined(_MSC_VER)
#include <compare>
#endif

export module vku:buffers;
export import :buffers.AllocatedBuffer;
export import :buffers.Buffer;
export import :buffers.MappedBuffer;