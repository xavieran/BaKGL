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

std::ostream& operator<<(std::ostream& os, const SaveFile& saveFile)
{
    os << "SaveFile{ " << saveFile.mIndex << ", " 
        << saveFile.mName << ", " << saveFile.mPath << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const SaveDirectory& saveDir)
{
    os << "SaveDir{ " << saveDir.mIndex << ", " 
        << saveDir.mName << ", " << saveDir.mSaves.size() << "}";
    return os;
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
    const std::string& saveName,
    bool isBookmark)
{
    mLogger.Debug() << __FUNCTION__ << "(" << saveDirectory << ", " << saveName << ")" << std::endl;
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
                static_cast<unsigned>(mDirectories.size() + 1),
                saveDirectory,
                {}};
            std::filesystem::create_directory(mSavePath / directory.GetPath());
            mLogger.Debug() << __FUNCTION__ << " Creating directory: " << mSavePath / directory.GetPath() << std::endl;
            return mDirectories.emplace_back(
                std::move(directory));
        }
    });

    if (isBookmark)
    {
        if (directory.mSaves.size() > 0
            && directory.mSaves.begin()->GetFilename() == "SAVE00.GAM")
        {
            mLogger.Debug() << __FUNCTION__ << " Found existing bookmark: " << *it << std::endl;
        }
        else
        {
            directory.mSaves.insert(directory.mSaves.begin(),
                SaveFile{0, "Bookmark", ((mSavePath / directory.GetPath()) / "SAVE00.GAM").string()});
            mLogger.Debug() << __FUNCTION__ << " Creating bookmark: " << *directory.mSaves.begin() << std::endl;
        }
        return *directory.mSaves.begin();
    }

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
        unsigned saveNumber = directory.mSaves.size() + 1;
        ss << "SAVE" << std::setw(2) << std::setfill('0') << saveNumber << ".GAM";
        return directory.mSaves.emplace_back(
            SaveFile{
                static_cast<unsigned>(directory.mSaves.size()),
                saveName,
                ((mSavePath / directory.GetPath()) / ss.str()).string()});
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
            const auto index = convertToInt(matches.str(1));
            const auto name = index == 0 ? "Bookmark" : LoadSaveName(fb);
            saveFiles.emplace_back(
                SaveFile{
                    index,
                    name,
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
    std::vector<SaveDirectory> saveDirs{};

    const auto gameDirectoryPath = Paths::Get().GetBakDirectoryPath() / "GAMES";
    if (!std::filesystem::exists(gameDirectoryPath))
    {
        mLogger.Info() << "Save game directory path: [" <<
            gameDirectoryPath << "] does not exist, creating it." << std::endl;
        try
        {
            std::filesystem::create_directory(gameDirectoryPath);
        }
        catch (const std::filesystem::filesystem_error& error)
        {
            mLogger.Error() << "Failed to create save game directory path: ["
                << gameDirectoryPath << "] " << error.what() << std::endl;
        }
        return saveDirs;
    }

    const auto saveDirectories = std::filesystem::directory_iterator{
        gameDirectoryPath};

    const auto dirSuffix = std::regex{".[Gg]([0-9]{2})$"};

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
