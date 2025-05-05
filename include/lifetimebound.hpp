#pragma once

#ifdef _MSC_VER
#define LIFETIMEBOUND [[msvc::lifetimebound]]
#elifdef __clang__
#define LIFETIMEBOUND [[clang::lifetimebound]]
#else
#define LIFETIMEBOUND
#endif