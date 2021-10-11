#pragma once

#ifdef NO_ASSERT
#define ASSERT(x) ;
#else
#define ASSERT(x) assert((x));
#endif
