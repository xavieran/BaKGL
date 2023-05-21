#pragma once

#include "bak/save.hpp"

#include "com/logger.hpp"
#include "com/path.hpp"

#include <algorithm>
#include <filesystem>
#include <regex>

namespace BAK {

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
    unsigned mIndex;
    std::string mName;
    std::vector<SaveFile> mSaves;
};

std::string LoadSaveName(FileBuffer& fb);
std::vector<SaveDirectory> MakeSaveDirectories();

class SaveManager
{
public:
    const SaveFile& MakeSave(
        const std::string& saveDirectory,
        const std::string& saveName)
    {
        const auto it = std::find_if(mDirectories.begin(), mDirectories.end(),
            [&](const auto& elem){
                return saveDirectory == GetDirectoryNameWithoutIndex(elem.mName);
            });

        auto& directory = std::invoke([&]() -> SaveDirectory& {
            if (it != mDirectories.end())
            {
                return *it;
            }
            else
            {
                std::stringstream ss{};
                ss << saveDirectory << std::setw(2) << std::setfill('0')<< ".G" << mDirectories.size();
                const auto directoryName = ss.str();
                std::filesystem::create_directory(mSavePath / directoryName);
                return mDirectories.emplace_back(
                    SaveDirectory{
                        static_cast<unsigned>(mDirectories.size()),
                        directoryName,
                        {}});
            }
        });


        const auto fileIt = std::find_if(directory.mSaves.begin(), directory.mSaves.end(),
            [&](const auto& elem){
                return saveName == elem.mName;
            });

        if (fileIt != directory.mSaves.end())
        {
            return *fileIt;
        }
        else
        {
            std::stringstream ss{};
            ss << "SAVE" << std::setw(2) << std::setfill('0') << directory.mSaves.size() << ".GAM";
            return directory.mSaves.emplace_back(
                SaveFile{
                    static_cast<unsigned>(directory.mSaves.size()),
                    saveName,
                    (mSavePath / directory.mName) / ss.str()});
        }
    }

private:
    std::string GetDirectoryNameWithoutIndex(const std::string& directory) const
    {
        return std::filesystem::path{directory}.stem();
    }

    std::filesystem::path mSavePath;
    std::vector<SaveDirectory> mDirectories;
};

}
