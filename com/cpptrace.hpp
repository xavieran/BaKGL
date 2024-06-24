#pragma once

#ifdef ENABLE_CPPTRACE
#include <cpptrace/cpptrace.hpp>

#define CPPTRACE(OSTREAM, PREFIX) (OSTREAM) << (PREFIX) << " -- " << cpptrace::stacktrace::current() << "\n"
#else
#define CPPTRACE(OSTREAM, PREFIX) {}
#endif
