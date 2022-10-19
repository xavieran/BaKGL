#include "com/demangle.hpp"

namespace com {

std::string demangle(const char* mangled)
{
      int status;
      std::unique_ptr<char[], void (*)(void*)> result(
        abi::__cxa_demangle(mangled, 0, 0, &status), std::free);
      return result.get() ? std::string(result.get()) : "error occurred";
}

}
