#pragma once

#include <version>

// If VKU_USE_STD_MODULE is not defined and standard library module is available, define it to enable the standard
// library module usage.
#if !defined(VKU_USE_STD_MODULE) && defined(__cpp_lib_modules)
#define VKU_USE_STD_MODULE
#endif