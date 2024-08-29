module;

#if !defined(VKU_USE_STD_MODULE) && defined(_MSC_VER)
#include <compare>
#endif

export module vku:images;
export import :images.Image;
export import :images.AllocatedImage;