#pragma once

#include "bak/file/fileBuffer.hpp"

namespace BAK::File {

class IDataBufferProvider
{
public:
    virtual FileBuffer* GetDataBuffer(const std::string& fileName) = 0;

    virtual ~IDataBufferProvider() {};
};

}
