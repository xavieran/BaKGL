#include "bak/file/util.hpp"

#include "com/logger.hpp"

namespace BAK::File {

unsigned GetStreamSize(std::ifstream& ifs)
{
    ifs.ignore( std::numeric_limits<std::streamsize>::max() );
    std::streamsize length = ifs.gcount();
    ifs.clear();
    ifs.seekg( 0, std::ios_base::beg );
    return static_cast<unsigned>(length);
}

FileBuffer CreateFileBuffer(const std::string& fileName)
{
    Logging::LogInfo(__FUNCTION__) << "Opening: " << fileName << std::endl;
    std::ifstream in{};
    in.open(fileName, std::ios::in | std::ios::binary);

    if (!in.good())
    {
        std::cerr << "Failed to open file: " << fileName<< std::endl;
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " OpenError!";
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }

    FileBuffer fb{GetStreamSize(in)};
    fb.Load(in);
    in.close();
    return fb;
}

}
