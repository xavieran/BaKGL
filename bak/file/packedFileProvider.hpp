#pragma once

#include "bak/file/IDataBufferProvider.hpp"

#include "com/logger.hpp"

#include <string>
#include <unordered_map>

namespace BAK::File {

class ResourceIndex;

class PackedFileDataProvider : public IDataBufferProvider
{
public:
    PackedFileDataProvider(IDataBufferProvider& dataProvider);

    PackedFileDataProvider(
        std::string resourceFile,
        std::string resourceIndex);

    FileBuffer* GetDataBuffer(const std::string& path) override;

    std::unordered_map<std::string, FileBuffer>& GetCache();
private:
    void Populate(
        BAK::FileBuffer* packedResource,
        const ResourceIndex& resourceIndex);

    std::unordered_map<std::string, FileBuffer> mCache;

    FileBuffer mResourceFileFb;
    const Logging::Logger& mLogger;
};

}
