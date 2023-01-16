#pragma once

#include "bak/fileBuffer.hpp"

#include <unordered_map>

namespace BAK {

class ResourceIndex
{
    static constexpr auto sFilename = "krondor.rmf";
    static constexpr auto sFilenameLength = 13;
public:
    struct ResourceIndexData
    {
        unsigned mHashKey;
        std::streamoff mOffset;
        std::size_t mSize;
    };

    ResourceIndex();

    const std::string& GetPackedResourceFile() const;
    const ResourceIndexData& GetResourceIndex(const std::string& resourceName) const;

private:
    std::string mPackedResourceName;
    std::unordered_map<std::string, ResourceIndexData> mResourceIndexMap;
};

class PackedResourceFile
{
public:
    PackedResourceFile();

    FileBuffer GetDataFile(const std::string& resourceName);
private:

    ResourceIndex mResourceIndex;
    FileBuffer mPackedResources;
};

}
