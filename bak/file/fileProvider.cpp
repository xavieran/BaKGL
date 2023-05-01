#include "bak/file/fileProvider.hpp"

#include "bak/file/util.hpp"

#include "com/logger.hpp"

namespace BAK::File {

FileDataProvider::FileDataProvider(const std::string& basePath)
:
    mBasePath{basePath}
{}

bool FileDataProvider::DataFileExists(const std::string& path) const
{
    return mCache.contains(path)
        || std::filesystem::exists(mBasePath / path);
}

FileBuffer* FileDataProvider::GetDataBuffer(const std::string& path)
{
    Logging::LogDebug("FileDataProvider") << "Searching for file: "
        << path << " in directory [" << mBasePath.string() << "]" << std::endl;

    if (DataFileExists(path))
    {
        if (!mCache.contains(path))
        {
            const auto [it, emplaced] = mCache.emplace(path, CreateFileBuffer(mBasePath / path));
            assert(emplaced);
        }
        return &mCache.at(path);
    }
    else
    {
        return nullptr;
    }
}

}
