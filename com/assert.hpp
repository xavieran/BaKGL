#pragma once

#include <cassert>

#ifdef NDEBUG
#define ASSERT(x) {};
#else
#define ASSERT(x) assert((x));
#endif
