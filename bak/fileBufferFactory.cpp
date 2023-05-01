#include "bak/fileBufferFactory.hpp"

#include "bak/file/util.hpp"

#include "com/logger.hpp"
#include "com/path.hpp"

#include <filesystem>

namespace BAK {

FileBufferFactory::FileBufferFactory()
:
    mDataPath{(std::filesystem::path{GetBakDirectory()} / "data").string()},
    mSavePath{(std::filesystem::path{GetBakDirectory()} / "save").string()},
    mDataFileProvider{{(std::filesystem::path{GetBakDirectory()} / "data").string()}}
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
        ss << "File not found: " << fileName << "\n";
        throw new std::runtime_error(ss.str());
    }
}

bool FileBufferFactory::DataBufferExists(const std::string& fileName)
{
    return mDataFileProvider.GetDataBuffer(fileName) != nullptr;
}

bool FileBufferFactory::SaveBufferExists(const std::string& fileName)
{
    // First look in the cwd
    if (std::filesystem::exists(fileName))
    {
        return true;
    }
    else
    {
        const auto realPath = std::filesystem::path{mSavePath} / fileName;
        return std::filesystem::exists(realPath.string());
    }

    return false;
}

FileBuffer FileBufferFactory::CreateSaveBuffer(const std::string& fileName)
{
    if (std::filesystem::exists(fileName))
        return File::CreateFileBuffer(fileName);
    else
    {
        const auto realPath = std::filesystem::path{mSavePath} / fileName;
        return File::CreateFileBuffer(realPath.string());
    }
}

}
