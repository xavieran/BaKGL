#include "bak/fileBufferFactory.hpp"

#include "bak/file/util.hpp"

#include "com/logger.hpp"
#include "com/path.hpp"

#include <filesystem>

namespace BAK {

FileBufferFactory::FileBufferFactory()
:
    mDataPath{(std::filesystem::path{Paths::Get().GetBakDirectory()} / "data").string()},
    mDataFileProvider{{(std::filesystem::path{Paths::Get().GetBakDirectory()} / "data").string()}}
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

FileBuffer FileBufferFactory::CreateDataBuffer(const std::string& fileName)
{
    auto* dataBuffer = mDataFileProvider.GetDataBuffer(fileName);
    if (dataBuffer != nullptr)
    {
        return dataBuffer->MakeSubBuffer(0, dataBuffer->GetSize());
    }
    else
    {
        std::stringstream ss{};
        ss << "File not found: " << fileName << std::endl;
        Logging::LogFatal("FileBufferFactory") << ss.str();
        throw new std::runtime_error(ss.str());
    }
}

bool FileBufferFactory::DataBufferExists(const std::string& fileName)
{
    return mDataFileProvider.GetDataBuffer(fileName) != nullptr;
}

FileBuffer FileBufferFactory::CreateSaveBuffer(const std::string& fileName)
{
    if (std::filesystem::exists(fileName))
    {
        return File::CreateFileBuffer(fileName);
    }
    else
    {
        return CreateDataBuffer(fileName);
    }
}

}
