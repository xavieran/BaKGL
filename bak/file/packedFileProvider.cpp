#include "bak/file/packedFileProvider.hpp"

#include "bak/file/packedResourceFile.hpp"

namespace BAK::File {

PackedFileDataProvider::PackedFileDataProvider(IDataBufferProvider& dataProvider)
:
    mCache{},
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

    for (const auto& index : resourceIndex.GetResourceIndex())
    {
        packedResource->Seek(index.mOffset);
        const auto resourceName = packedResource->GetString(ResourceIndex::sFilenameLength);
        const auto resourceSize = packedResource->GetUint32LE();
        const auto [it, emplaced] = mCache.emplace(
            resourceName,
            packedResource->MakeSubBuffer(index.mOffset + ResourceIndex::sFilenameLength + 4, resourceSize));

        mLogger.Debug() << "Resource: " << resourceName << " hash: " << std::hex << index.mHashKey
            << std::dec << " offset: " << index.mOffset << " size: " << resourceSize << "\n";
    }
}

FileBuffer* PackedFileDataProvider::GetDataBuffer(const std::string& fileName)
{
    mLogger.Debug() << "Searching for file: " << fileName << std::endl;
    if (mCache.contains(fileName))
    {
        return &mCache.at(fileName);
    }

    return nullptr;
}

}
