#pragma once

#include <string>
#ifndef _MSC_VER
#include <cxxabi.h>
#endif
#include <memory>

namespace com {

std::string demangle(const char* mangled);

}

