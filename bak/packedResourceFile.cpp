#include "bak/packedResourceFile.hpp"

#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"

namespace BAK {

ResourceIndex::ResourceIndex()
:
    mPackedResourceName{},
    mResourceIndexMap{}
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer(sFilename);
    const auto rmfMajorVersion = fb.GetUint32LE();
    const auto rmfMinorVersion = fb.GetUint16LE();

    mPackedResourceName = fb.GetString(sFilenameLength);
    const auto numPackedResources = fb.GetUint16LE();
    Logging::LogSpam("BAK::ResourceIndex") << "RMF Version (" << rmfMajorVersion << ", " << rmfMinorVersion 
        << ") ResourceFile: " << mPackedResourceName << " Resources: " 
        << numPackedResources << "\n";

    auto packedResourceFb = FileBufferFactory::Get().CreateDataBuffer(mPackedResourceName);

    for (unsigned i = 0; i < numPackedResources; i++)
    {
        const unsigned hashKey = fb.GetUint32LE();
        const std::streamoff offset = fb.GetUint32LE();
        packedResourceFb.Seek(offset);
        const auto resourceName = packedResourceFb.GetString(sFilenameLength);
        const auto resourceSize = packedResourceFb.GetUint32LE();
        Logging::LogSpam("BAK::ResourceIndex") << "Resource: " << resourceName << " hash: " << std::hex << hashKey
            << std::dec << " offset: " << offset << " size: " << resourceSize << "\n";
        mResourceIndexMap.emplace(
            resourceName,
            ResourceIndexData{
                hashKey,
                offset + sFilenameLength + 4,
                resourceSize});
    }
}

const std::string& ResourceIndex::GetPackedResourceFile() const
{
    return mPackedResourceName;
}

const ResourceIndex::ResourceIndexData& ResourceIndex::GetResourceIndex(const std::string& resourceName) const
{
    return mResourceIndexMap.find(resourceName)->second;
}


PackedResourceFile::PackedResourceFile()
:
    mResourceIndex{},
    mPackedResources{
        FileBufferFactory::Get().CreateDataBuffer(
            mResourceIndex.GetPackedResourceFile())}
{
}

FileBuffer PackedResourceFile::GetDataFile(const std::string& resourceName)
{
    const auto& resourceIndex = mResourceIndex.GetResourceIndex(resourceName);
    return mPackedResources.MakeSubBuffer(resourceIndex.mOffset, resourceIndex.mSize);
}

}
