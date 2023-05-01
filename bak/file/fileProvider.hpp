#pragma once

#include "bak/fileBufferFactory.hpp"
#include "bak/file/IDataBufferProvider.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>

namespace BAK::File {

class FileDataProvider : public IDataBufferProvider
{
public:
    FileDataProvider(const std::string& basePath);

    FileBuffer* GetDataBuffer(const std::string& path) override;

private:
    bool DataFileExists(const std::string& path) const;

    std::unordered_map<std::string, FileBuffer> mCache;
    std::filesystem::path mBasePath;
};

}
