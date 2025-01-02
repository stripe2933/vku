/** @file rendering/mod.cppm
 */

module;

#if !defined(VKU_USE_STD_MODULE) && defined(_MSC_VER)
#include <compare>
#include <forward_list>
#endif

export module vku:rendering;

export import :rendering.Attachment;
export import :rendering.MultisampleAttachment;
export import :rendering.AttachmentGroup;
export import :rendering.MultisampleAttachmentGroup;