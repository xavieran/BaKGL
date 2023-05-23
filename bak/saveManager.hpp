#pragma once

#include "bak/save.hpp"

#include "com/logger.hpp"
#include "com/path.hpp"

#include <algorithm>
#include <filesystem>
#include <regex>

namespace BAK {

std::string LoadSaveName(FileBuffer& fb);

class SaveFile
{
public:
    unsigned mIndex;
    std::string mName;
    std::string mPath;
};

class SaveDirectory
{
public:
    std::string GetPath() const
    {
        std::stringstream ss{};
        ss << mName << std::setw(2) << std::setfill('0')<< ".G" << mIndex;
        return ss.str();
    }

    std::string GetName() const
    {
        return mName;
    }

    unsigned mIndex;
    std::string mName;
    std::vector<SaveFile> mSaves;
};

std::ostream& operator<<(auto& os, const SaveDirectory& saveDir)
{
    os << "SaveDir{ " << saveDir.mIndex << ", " 
        << saveDir.mName << ", " << saveDir.mSaves.size() << "}";
    return os;
}

class SaveManager
{
public:
    SaveManager(const std::string& savePath);

    const std::vector<SaveDirectory>& GetSaves() const;
    void RefreshSaves();

    void RemoveDirectory(unsigned index);
    void RemoveSave(unsigned directory, unsigned save);

    const SaveFile& MakeSave(
        const std::string& saveDirectory,
        const std::string& saveName);
private:
    std::vector<SaveFile> MakeSaveFiles(std::filesystem::path saveDir);
    std::vector<SaveDirectory> MakeSaveDirectories();

    std::filesystem::path mSavePath;
    std::vector<SaveDirectory> mDirectories;

    const Logging::Logger& mLogger;
};

}
