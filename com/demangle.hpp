#pragma once

#include <string>
#include <cxxabi.h>
#include <memory>

namespace com {

std::string demangle(const char* mangled);

}

