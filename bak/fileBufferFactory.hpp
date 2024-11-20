#pragma once

#include "bak/file/aggregateFileProvider.hpp"

#include <string>

namespace BAK {

class FileBuffer;

class FileBufferFactory
{
public:
    static FileBufferFactory& Get();

    void SetDataPath(const std::string&);
    void SetSavePath(const std::string&);

    bool DataBufferExists(const std::string& path);
    FileBuffer CreateDataBuffer(const std::string& path);
    bool SaveBufferExists(const std::string& path);
    FileBuffer CreateSaveBuffer(const std::string& path);
    FileBuffer CreateFileBuffer(const std::string& path);

private:
    FileBufferFactory();

    FileBufferFactory& operator=(const FileBufferFactory&) noexcept = delete;
    FileBufferFactory(const FileBufferFactory&) noexcept = delete;
    FileBufferFactory& operator=(FileBufferFactory&&) noexcept = delete;
    FileBufferFactory(FileBufferFactory&&) noexcept = delete;

    std::string mDataPath;
    std::string mSavePath;
    File::AggregateFileProvider mDataFileProvider;
};

}
