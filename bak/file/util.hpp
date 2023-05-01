#include "bak/fileBufferFactory.hpp"

namespace BAK::File {

unsigned GetStreamSize(std::ifstream& ifs);

FileBuffer CreateFileBuffer(const std::string& fileName);

}
