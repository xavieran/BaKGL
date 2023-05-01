#include "bak/file/aggregateFileProvider.hpp"

#include "bak/file/fileProvider.hpp"
#include "bak/file/packedFileProvider.hpp"

namespace BAK::File {

AggregateFileProvider::AggregateFileProvider(const std::vector<std::string>& searchPaths)
:
    mProviders{}
{
    mProviders.emplace_back(
        std::make_unique<FileDataProvider>("."));

    for (const auto& path : searchPaths)
    {
        mProviders.emplace_back(
            std::make_unique<FileDataProvider>(path));
    }

    mProviders.emplace(
        // Uncomment if we want to search in cwd first...
        //std::next(mProviders.begin()),
        mProviders.begin(),
        std::make_unique<PackedFileDataProvider>(*this));
}

FileBuffer* AggregateFileProvider::GetDataBuffer(const std::string& fileName)
{
    for (auto& provider : mProviders)
    {
        assert(provider);
        auto* fb = provider->GetDataBuffer(fileName);
        if (fb != nullptr)
        {
            return fb;
        }
    }

    return nullptr;
}

}
