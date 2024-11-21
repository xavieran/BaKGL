#include "bak/file/packedResourceFile.hpp"

#include "bak/file/fileBuffer.hpp"

#include "com/logger.hpp"

namespace BAK::File {

ResourceIndex::ResourceIndex(
    FileBuffer& resourceIndex)
:
    mPackedResourceName{},
    mResourceIndexData{}
{
    const auto rmfMajorVersion = resourceIndex.GetUint32LE();
    const auto rmfMinorVersion = resourceIndex.GetUint16LE();

    mPackedResourceName = resourceIndex.GetString(sFilenameLength);
    const auto numPackedResources = resourceIndex.GetUint16LE();
    Logging::LogDebug("BAK::ResourceIndex") << "RMF Version (" << rmfMajorVersion << ", " << rmfMinorVersion 
        << ") ResourceFile: " << mPackedResourceName << " Resources: " 
        << numPackedResources << "\n";

    for (unsigned i = 0; i < numPackedResources; i++)
    {
        const unsigned hashKey = resourceIndex.GetUint32LE();
        const std::streamoff offset = resourceIndex.GetUint32LE();
        mResourceIndexData.emplace_back(ResourceIndexData{hashKey, offset});
        //packedResource.Seek(offset);
        //const auto resourceName = packedResource.GetString(sFilenameLength);
        //const auto resourceSize = packedResource.GetUint32LE();
        //Logging::LogDebug("BAK::ResourceIndex") << "Resource: " << resourceName << " hash: " << std::hex << hashKey
        //    << std::dec << " offset: " << offset << " size: " << resourceSize << "\n";
        //mResourceIndexMap.emplace(
        //    resourceName,
        //    ResourceIndexData{
        //        hashKey,
        //        offset + sFilenameLength + 4,
        //        resourceSize});
    }
}

const std::string& ResourceIndex::GetPackedResourceFile() const
{
    return mPackedResourceName;
}

const std::vector<ResourceIndex::ResourceIndexData>& ResourceIndex::GetResourceIndex() const
{
    return mResourceIndexData;
}

}
