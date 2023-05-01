#pragma once

#include "bak/file/IDataBufferProvider.hpp"

#include <memory>
#include <vector>

namespace BAK::File {

class AggregateFileProvider : public IDataBufferProvider
{
public:
    AggregateFileProvider(const std::vector<std::string>& searchPaths);

    FileBuffer* GetDataBuffer(const std::string& fileName) override;

private:
    std::vector<std::unique_ptr<IDataBufferProvider>> mProviders;
};

}
