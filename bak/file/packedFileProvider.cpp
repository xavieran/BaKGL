#include "bak/file/packedFileProvider.hpp"

#include "bak/file/packedResourceFile.hpp"

#include "bak/file/util.hpp"

#include <array>
#include <cstdint>
#include <cstring>

namespace BAK::File {

namespace {

enum class BakVersion { V101, V102, Unknown };

constexpr std::array<std::uint8_t, 32> sV101RmfSignature = {{
    0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x6b, 0x72, 0x6f, 0x6e, 0x64, 0x6f, 0x72, 0x2e, 0x30, 0x30,
    0x31, 0x00, 0x00, 0xe6, 0x06, 0x34, 0x59, 0x42, 0x64, 0x00, 0x00, 0x00, 0x00, 0x34, 0x59, 0x52
}};

constexpr std::array<std::uint8_t, 32> sV102RmfSignature = {{
    0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x6b, 0x72, 0x6f, 0x6e, 0x64, 0x6f, 0x72, 0x2e, 0x30, 0x30,
    0x31, 0x00, 0x00, 0xe6, 0x06, 0x07, 0xb8, 0x42, 0x33, 0x00, 0x00, 0x00, 0x00, 0x47, 0x8b, 0x43
}};

constexpr std::array<std::uint8_t, 32> sV101PackedSignature = {{
    0x43, 0x31, 0x31, 0x2e, 0x41, 0x44, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00,
    0x00, 0x56, 0x45, 0x52, 0x3a, 0x05, 0x00, 0x00, 0x00, 0x31, 0x2e, 0x30, 0x39, 0x00, 0x41, 0x44
}};

constexpr std::array<std::uint8_t, 32> sV102PackedSignature = {{
    0x41, 0x43, 0x54, 0x30, 0x30, 0x31, 0x2e, 0x42, 0x4d, 0x58, 0x00, 0x00, 0x00, 0xea, 0x11, 0x00,
    0x00, 0x66, 0x10, 0x00, 0x00, 0x01, 0x00, 0xd1, 0x11, 0xf9, 0x15, 0x00, 0x00, 0xf9, 0x15, 0xe0
}};

bool MatchesSignature(BAK::FileBuffer& buffer, const std::array<std::uint8_t, 32>& signature)
{
    if (buffer.GetSize() < signature.size()) return false;
    buffer.Seek(0);
    return std::memcmp(buffer.GetCurrent(), signature.data(), signature.size()) == 0;
}

BakVersion GetRmfVersion(BAK::FileBuffer& buffer)
{
    if (MatchesSignature(buffer, sV101RmfSignature)) return BakVersion::V101;
    if (MatchesSignature(buffer, sV102RmfSignature)) return BakVersion::V102;
    return BakVersion::Unknown;
}

BakVersion GetPackedVersion(BAK::FileBuffer& buffer)
{
    if (MatchesSignature(buffer, sV101PackedSignature)) return BakVersion::V101;
    if (MatchesSignature(buffer, sV102PackedSignature)) return BakVersion::V102;
    return BakVersion::Unknown;
}

std::string_view ToString(BakVersion version)
{
    switch (version)
    {
        case BakVersion::V101: return "1.01";
        case BakVersion::V102: return "1.02";
        default: return "Unknown";
    }
}

bool ValidateSignatures(BAK::FileBuffer& rmfBuffer, BAK::FileBuffer& packedBuffer, const Logging::Logger& logger)
{
    auto rmfPosition = rmfBuffer.Tell();
    auto packedPosition = packedBuffer.Tell();
    assert(packedPosition == 0);
    const auto rmfVersion = GetRmfVersion(rmfBuffer);
    const auto packedVersion = GetPackedVersion(packedBuffer);
    rmfBuffer.Seek(rmfPosition);
    packedPosition = packedBuffer.Tell();
    assert(packedPosition == 0);

    logger.Info() << "RMF file (krondor.rmf) version: " << ToString(rmfVersion) << "\n";
    logger.Info() << "Packed resource file (krondor.001) version: " << ToString(packedVersion) << "\n";

    if (rmfVersion == BakVersion::Unknown || packedVersion == BakVersion::Unknown)
    {
        logger.Error() << "Unable to identify resource file version.\n";
        logger.Error() << "Please ensure krondor.rmf and krondor.001 are from a supported game version.\n";
        logger.Error() << "Continuing because this might be a non-english BaK, but this might not work!\n";
        return true;
    }

    if (rmfVersion != packedVersion)
    {
        logger.Warn() << "Resource file version mismatch: RMF is " << ToString(rmfVersion)
            << " but packed resource is " << ToString(packedVersion) << "\n";
        logger.Warn() << "Please ensure krondor.rmf and krondor.001 are from the same game version." << "\n";
        return false;
    }

    logger.Info() << "Validated resource files match version " << ToString(rmfVersion) << "\n";
    return true;
}

}

PackedFileDataProvider::PackedFileDataProvider(IDataBufferProvider& dataProvider)
:
    mCache{},
    mResourceFileFb{0},
    mLogger{Logging::LogState::GetLogger("PackedFileDataProvider")}
{
    auto* resourceIndexFb = dataProvider.GetDataBuffer(ResourceIndex::sFilename);
    if (resourceIndexFb == nullptr)
    {
        mLogger.Warn() << "Could not find resource index file [" << ResourceIndex::sFilename
            << "]. Will not use packed resource file for data.\n";
        return;
    }

    auto resourceIndex = ResourceIndex{*resourceIndexFb};

    auto* packedResource = dataProvider.GetDataBuffer(resourceIndex.GetPackedResourceFile());

    if (packedResource == nullptr)
    {
        mLogger.Warn() << "Could not find packed resource file [" << resourceIndex.GetPackedResourceFile()
            << "]. Will not use packed resource file for data.\n";
        return;
    }

    if (!ValidateSignatures(*resourceIndexFb, *packedResource, mLogger))
    {
        mLogger.Error() << "Resource file version mismatch. Will not use packed resource file for data.\n";
        return;
    }

    Populate(packedResource, resourceIndex);
}

PackedFileDataProvider::PackedFileDataProvider(
    std::string resourceFile,
    std::string resourceIndexFile)
:
    mCache{},
    mResourceFileFb{CreateFileBuffer(resourceFile)},
    mLogger{Logging::LogState::GetLogger("PackedFileDataProvider")}
{
    FileBuffer resourceIndexFb = CreateFileBuffer(resourceIndexFile);

    if (!ValidateSignatures(resourceIndexFb, mResourceFileFb, mLogger))
    {
        mLogger.Error() << "Resource file version mismatch. Will not use packed resource file for data." << "\n";
        return;
    }

    auto resourceIndex = ResourceIndex{resourceIndexFb};
    Populate(&mResourceFileFb, resourceIndex);
}

void PackedFileDataProvider::Populate(
    BAK::FileBuffer* packedResource,
    const ResourceIndex& resourceIndex) {

    const auto packedSize = packedResource->GetSize();

    for (const auto& index : resourceIndex.GetResourceIndex())
    {
        if (index.mOffset + ResourceIndex::sFilenameLength + 4 > packedSize)
        {
            mLogger.Error() << "Resource at offset " << index.mOffset
                << " extends beyond packed resource file (size: " << packedSize
                << "). Skipping.\n";
            continue;
        }

        packedResource->Seek(index.mOffset);
        const auto resourceName = packedResource->GetString(ResourceIndex::sFilenameLength);

        bool validName = !resourceName.empty();
        for (auto c : resourceName)
        {
            if (c < 0x20 || c > 0x7E)
            {
                validName = false;
                break;
            }
        }

        if (!validName)
        {
            mLogger.Error() << "Invalid resource name at offset " << index.mOffset
                << ". Resource index may not match packed resource file. Skipping.\n";
            continue;
        }

        const auto resourceSize = packedResource->GetUint32LE();

        if (index.mOffset + ResourceIndex::sFilenameLength + 4 + resourceSize > packedSize)
        {
            mLogger.Error() << "Resource '" << resourceName << "' at offset " << index.mOffset
                << " with size " << resourceSize
                << " extends beyond packed resource file (size: " << packedSize
                << "). Resource index may not match packed resource file. Skipping.\n";
            continue;
        }

        const auto [it, emplaced] = mCache.emplace(
            resourceName,
            packedResource->MakeSubBuffer(index.mOffset + ResourceIndex::sFilenameLength + 4, resourceSize));

        mLogger.Spam() << "Resource: " << resourceName << " hash: " << std::hex << index.mHashKey
            << std::dec << " offset: " << index.mOffset << " size: " << resourceSize << "\n";
    }
}

FileBuffer* PackedFileDataProvider::GetDataBuffer(const std::string& fileName)
{
    mLogger.Spam() << "Searching for file: " << fileName << "\n";
    if (mCache.contains(fileName))
    {
        return &mCache.at(fileName);
    }

    return nullptr;
}

std::unordered_map<std::string, FileBuffer>& PackedFileDataProvider::GetCache()
{
    return mCache;
}

}
