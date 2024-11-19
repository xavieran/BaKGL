#include "bak/file/packedFileProvider.hpp"

#include "bak/file/packedResourceFile.hpp"

#include "bak/file/util.hpp"

namespace BAK::File {

PackedFileDataProvider::PackedFileDataProvider(IDataBufferProvider& dataProvider)
:
    mCache{},
    mResourceFileFb{0},
    mLogger{Logging::LogState::GetLogger("PackedFileDataProvider")}
{
    auto* resourceIndexFb = dataProvider.GetDataBuffer(ResourceIndex::sFilename);
    if (resourceIndexFb == nullptr)
    {
        mLogger.Warn() << "Could not find resource index file [" << ResourceIndex::sFilename
            << "]. Will not use packed resource file for data." << std::endl;
        return;
    }

    auto resourceIndex = ResourceIndex{*resourceIndexFb};

    auto* packedResource = dataProvider.GetDataBuffer(resourceIndex.GetPackedResourceFile());

    if (packedResource == nullptr)
    {
        mLogger.Warn() << "Could not find packed resource file [" << resourceIndex.GetPackedResourceFile()
            << "]. Will not use packed resource file for data." << std::endl;
        return;
    }

    Populate(packedResource, resourceIndex);
}

PackedFileDataProvider::PackedFileDataProvider(
    std::string resourceFile,
    std::string resourceIndexFile)
:
    mCache{},
    mResourceFileFb{CreateFileBuffer(resourceFile)},
    mLogger{Logging::LogState::GetLogger("PackedFileDataProvider")}
{
    FileBuffer resourceIndexFb = CreateFileBuffer(resourceIndexFile);
    auto resourceIndex = ResourceIndex{resourceIndexFb};
    Populate(&mResourceFileFb, resourceIndex);
}

void PackedFileDataProvider::Populate(
    BAK::FileBuffer* packedResource,
    const ResourceIndex& resourceIndex) {

    for (const auto& index : resourceIndex.GetResourceIndex())
    {
        packedResource->Seek(index.mOffset);
        const auto resourceName = packedResource->GetString(ResourceIndex::sFilenameLength);
        const auto resourceSize = packedResource->GetUint32LE();
        const auto [it, emplaced] = mCache.emplace(
            resourceName,
            packedResource->MakeSubBuffer(index.mOffset + ResourceIndex::sFilenameLength + 4, resourceSize));

        mLogger.Spam() << "Resource: " << resourceName << " hash: " << std::hex << index.mHashKey
            << std::dec << " offset: " << index.mOffset << " size: " << resourceSize << "\n";
    }
}

FileBuffer* PackedFileDataProvider::GetDataBuffer(const std::string& fileName)
{
    mLogger.Spam() << "Searching for file: " << fileName << std::endl;
    if (mCache.contains(fileName))
    {
        return &mCache.at(fileName);
    }

    return nullptr;
}

std::unordered_map<std::string, FileBuffer>& PackedFileDataProvider::GetCache()
{
    return mCache;
}

}
