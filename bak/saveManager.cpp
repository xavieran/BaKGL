#include "bak/saveManager.hpp"

#include "bak/file/util.hpp"

namespace BAK {

unsigned convertToInt(const std::string& s)
{
    std::stringstream ss{};
    ss << s;
    unsigned index{};
    ss >> index;
    return index;
}

std::string LoadSaveName(FileBuffer& fb)
{
    fb.Seek(0);
    return fb.GetString(30);
}

SaveManager::SaveManager(
    const std::string& savePath)
:
    mSavePath{savePath},
    mDirectories{},
    mLogger{Logging::LogState::GetLogger("BAK::SaveManager")}
{}

const std::vector<SaveDirectory>& SaveManager::GetSaves() const
{
    return mDirectories;
}

void SaveManager::RefreshSaves()
{
    mDirectories = MakeSaveDirectories();
}

void SaveManager::RemoveDirectory(unsigned index)
{
    mLogger.Info() << "Removing save directory: " << mDirectories.at(index) << "\n";
    std::filesystem::remove_all(mSavePath / mDirectories.at(index).GetPath());
    RefreshSaves();
}

void SaveManager::RemoveSave(unsigned directory, unsigned save)
{
    mLogger.Info() << "Removing save file: "
        << mDirectories.at(directory).mSaves.at(save).mPath << "\n";
    std::filesystem::remove(mDirectories.at(directory).mSaves.at(save).mPath);
    RefreshSaves();
}

const SaveFile& SaveManager::MakeSave(
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

std::vector<SaveFile> SaveManager::MakeSaveFiles(std::filesystem::path saveDir)
{
    const auto saveSuffix = std::regex{"[Ss][Aa][Vv][Ee]([0-9]{2}).[Gg][Aa][mM]$"};
    const auto saveFileDir = std::filesystem::directory_iterator{saveDir};

    std::vector<SaveFile> saveFiles{};
    for (const auto& save : saveFileDir)
    {
        const auto saveName = save.path().filename().string();
        Logging::LogDebug(__FUNCTION__) << "Save: " << saveName << " matches: " << std::regex_search(saveName, saveSuffix) << std::endl;
        std::smatch matches{};
        std::regex_search(saveName, matches, saveSuffix);
        if (matches.size() > 0)
        {
            auto fb = File::CreateFileBuffer(save.path().string());
            saveFiles.emplace_back(
                SaveFile{
                    convertToInt(matches.str(1)),
                    LoadSaveName(fb),
                    save.path().string()});
        }
    }

    std::sort(
        saveFiles.begin(), saveFiles.end(),
        [](const auto& lhs, const auto& rhs){ return lhs.mIndex < rhs.mIndex; });

    return saveFiles;
}

std::vector<SaveDirectory> SaveManager::MakeSaveDirectories()
{
    const auto dirSuffix = std::regex{".[Gg]([0-9]{2})$"};

    const auto saveDirectories = std::filesystem::directory_iterator{
        GetBakDirectoryPath() / "GAMES"};

    std::vector<SaveDirectory> saveDirs{};
    for (const auto& directory : saveDirectories)
    {
        const auto dirName = directory.path().filename().string();
        Logging::LogDebug(__FUNCTION__) << "Directory: " << dirName << " matches: " << std::regex_search(dirName, dirSuffix) << std::endl;
        std::smatch matches{};
        std::regex_search(dirName, matches, dirSuffix);
        if (matches.size() > 0)
        {
            saveDirs.emplace_back(
                SaveDirectory{
                    convertToInt(matches.str(1)),
                    directory.path().stem().string(),
                    MakeSaveFiles(directory.path())});
        }
    }
    std::sort(
        saveDirs.begin(), saveDirs.end(),
        [](const auto& lhs, const auto& rhs){ return lhs.mIndex < rhs.mIndex; });

    return saveDirs;
}

}
