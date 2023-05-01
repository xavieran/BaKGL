#pragma once

#include "bak/fileBufferFactory.hpp"

#include <unordered_map>

namespace BAK::File {

class ResourceIndex
{
public:
    static constexpr auto sFilename = "krondor.rmf";
    static constexpr auto sFilenameLength = 13;

    struct ResourceIndexData
    {
        unsigned mHashKey;
        std::streamoff mOffset;
    };

    ResourceIndex(FileBuffer&);

    const std::string& GetPackedResourceFile() const;
    unsigned GetResourceSize() const;
    const std::vector<ResourceIndex::ResourceIndexData>&
        GetResourceIndex() const;

private:
    std::string mPackedResourceName;
    std::vector<ResourceIndexData> mResourceIndexData;
};

}
