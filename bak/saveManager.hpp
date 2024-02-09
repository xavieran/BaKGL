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

    std::string GetFilename() const
    {
        return std::filesystem::path{mPath}.filename().string();
    }
};

std::ostream& operator<<(std::ostream& os, const SaveFile&);

class SaveDirectory
{
public:
    std::string GetPath() const
    {
        std::stringstream ss{};
        ss << mName << ".G" << std::setw(2) << std::setfill('0') << mIndex;
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

std::ostream& operator<<(std::ostream&, const SaveDirectory&);

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
        const std::string& saveName,
        bool isBookmark);
private:
    std::vector<SaveFile> MakeSaveFiles(std::filesystem::path saveDir);
    std::vector<SaveDirectory> MakeSaveDirectories();

    std::filesystem::path mSavePath;
    std::vector<SaveDirectory> mDirectories;

    const Logging::Logger& mLogger;
};

}
