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

std::string LoadSaveName(FileBuffer& fb);
std::vector<SaveDirectory> MakeSaveDirectories();

class SaveManager
{
public:
    SaveManager(
        const std::string& savePath)
    :
        mSavePath{savePath},
        mDirectories{},
        mLogger{Logging::LogState::GetLogger("BAK::SaveManager")}
    {}

    const std::vector<SaveDirectory>& GetSaves() const
    {
        return mDirectories;
    }

    void RefreshSaves()
    {
        mDirectories = MakeSaveDirectories();
    }

    void RemoveDirectory(unsigned index)
    {
        mLogger.Info() << "Removing save directory: " << mDirectories.at(index) << "\n";
        std::filesystem::remove_all(mSavePath / mDirectories.at(index).GetPath());
        RefreshSaves();
    }

    void RemoveSave(unsigned directory, unsigned save)
    {
        mLogger.Info() << "Removing save file: "
            << mDirectories.at(directory).mSaves.at(save).mPath << "\n";
        std::filesystem::remove(mDirectories.at(directory).mSaves.at(save).mPath);
        RefreshSaves();
    }

    const SaveFile& MakeSave(
        const std::string& saveDirectory,
        const std::string& saveName)
    {
        mLogger.Debug() << __FUNCTION__ << "(" << saveDirectory << ", " << saveName << std::endl;
        const auto it = std::find_if(mDirectories.begin(), mDirectories.end(),
            [&](const auto& elem){
                return saveDirectory == elem.mName;
            });

        auto& directory = std::invoke([&]() -> SaveDirectory& {
            if (it != mDirectories.end())
            {
                mLogger.Debug() << __FUNCTION__ << " Found existing save directory: " << *it << std::endl;
                return *it;
            }
            else
            {
                auto directory = SaveDirectory{
                    static_cast<unsigned>(mDirectories.size()),
                    saveDirectory,
                    {}};
                std::filesystem::create_directory(mSavePath / directory.GetPath());
                mLogger.Debug() << __FUNCTION__ << " Creating directory: " << mSavePath / directory.GetPath() << std::endl;
                return mDirectories.emplace_back(
                    std::move(directory));
            }
        });

        mLogger.Debug() << __FUNCTION__ << " SaveDir: " << directory << std::endl;
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
                    (mSavePath / directory.GetPath()) / ss.str()});
        }
    }

private:
    std::filesystem::path mSavePath;
    std::vector<SaveDirectory> mDirectories;

    const Logging::Logger& mLogger;
};

}
