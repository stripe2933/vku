#pragma once

#define VKU_FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)
#define VKU_INDEX_SEQ(Is, N, ...) [&]<std::size_t ...Is>(std::index_sequence<Is...>) __VA_ARGS__ (std::make_index_sequence<N>{})
#define VKU_ARRAY_OF(N, ...) VKU_INDEX_SEQ(Is, N, { return std::array { (Is, __VA_ARGS__)... }; })