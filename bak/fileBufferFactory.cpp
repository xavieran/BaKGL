#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"
#include "com/path.hpp"

#include <filesystem>

namespace BAK {

unsigned GetStreamSize(std::ifstream& ifs)
{
    ifs.ignore( std::numeric_limits<std::streamsize>::max() );
    std::streamsize length = ifs.gcount();
    ifs.clear();
    ifs.seekg( 0, std::ios_base::beg );
    return static_cast<unsigned>(length);
}

FileBufferFactory::FileBufferFactory()
:
    mDataPath{(std::filesystem::path{GetBakDirectory()} / "data").string()},
    mSavePath{(std::filesystem::path{GetBakDirectory()} / "save").string()}
{}

FileBufferFactory& FileBufferFactory::Get()
{
    static FileBufferFactory factory{};
    return factory;
}

void FileBufferFactory::SetDataPath(const std::string& dataPath)
{
    mDataPath = dataPath;
}

void FileBufferFactory::SetSavePath(const std::string& savePath)
{
    mSavePath = savePath;
}

FileBuffer FileBufferFactory::CreateDataBuffer(const std::string& path)
{
    // First look in the cwd
    if (std::filesystem::exists(path))
    {
        return CreateFileBuffer(path);
    }
    else
    {
        const auto realPath = std::filesystem::path{mDataPath} / path;
        return CreateFileBuffer(realPath.string());
    }
}

bool FileBufferFactory::DataBufferExists(const std::string& path)
{
    // First look in the cwd
    if (std::filesystem::exists(path))
    {
        return true;
    }
    else
    {
        const auto realPath = std::filesystem::path{mDataPath} / path;
        return std::filesystem::exists(realPath.string());
    }

    return false;
}

bool FileBufferFactory::SaveBufferExists(const std::string& path)
{
    // First look in the cwd
    if (std::filesystem::exists(path))
    {
        return true;
    }
    else
    {
        const auto realPath = std::filesystem::path{mSavePath} / path;
        return std::filesystem::exists(realPath.string());
    }

    return false;
}

FileBuffer FileBufferFactory::CreateSaveBuffer(const std::string& path)
{
    if (std::filesystem::exists(path))
        return CreateFileBuffer(path);
    else
    {
        const auto realPath = std::filesystem::path{mSavePath} / path;
        return CreateFileBuffer(realPath.string());
    }
}

FileBuffer FileBufferFactory::CreateFileBuffer(const std::string& path)
{
    Logging::LogInfo(__FUNCTION__) << "Opening: " << path << std::endl;
    std::ifstream in{};
    in.open(path, std::ios::in | std::ios::binary);
    if (!in.good())
    {
        std::cerr << "Failed to open file: " << path << std::endl;
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
