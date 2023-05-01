#pragma once

#include "bak/file/IDataBufferProvider.hpp"

#include "com/logger.hpp"

#include <string>
#include <unordered_map>

namespace BAK::File {

class PackedFileDataProvider : public IDataBufferProvider
{
public:
    PackedFileDataProvider(IDataBufferProvider& dataProvider);

    FileBuffer* GetDataBuffer(const std::string& path) override;

private:

    std::unordered_map<std::string, FileBuffer> mCache;
    const Logging::Logger& mLogger;
};

}
