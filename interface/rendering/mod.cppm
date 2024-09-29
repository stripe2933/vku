module;

#include <macros.hpp>

#if !defined(VKU_USE_STD_MODULE) && defined(_MSC_VER)
#include <compare>
#endif

export module vku:rendering;

export import :rendering.Attachment;
export import :rendering.MsaaAttachment;
export import :rendering.AttachmentGroup;
export import :rendering.MsaaAttachmentGroup;