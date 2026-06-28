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

    // Check is_open() rather than good(): on mingw-w64, a successful open() leaves eofbit set
    // (so good() == false even though the file is open). Clear the spurious bit and the stream
    // reads normally (byte-identical to a stdio read).
    if (!in.is_open())
    {
        std::cerr << "Failed to open file: " << fileName<< std::endl;
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " OpenError!";
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }
    in.clear();

    FileBuffer fb{GetStreamSize(in)};
    fb.Load(in);
    in.close();
    return fb;
}

}
