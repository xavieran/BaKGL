#include "bak/saveFile.hpp"

namespace BAK {

std::vector<SaveFile> MakeSaveFiles(std::filesystem::path saveDir)
{
    const auto saveSuffix = std::regex{"[Ss][Aa][Vv][Ee][0-9]{2}.[Gg][Aa][mM]$"};
    const auto saveFileDir = std::filesystem::directory_iterator{saveDir};

    std::vector<SaveFile> saveFiles{};
    for (const auto& save : saveFileDir)
    {
        const auto saveName = save.path().filename().string();
        Logging::LogDebug(__FUNCTION__) << "Save: " << saveName << " matches: " << std::regex_search(saveName, saveSuffix) << std::endl;
        if (std::regex_search(saveName, saveSuffix))
        {
            auto fb = FileBufferFactory::Get().CreateFileBuffer(save.path().string());
            saveFiles.emplace_back(SaveFile{0, LoadSaveName(fb), save.path().string()});
        }
    }
    return saveFiles;
}

std::vector<SaveDirectory> MakeSaveDirectories()
{
    const auto dirSuffix = std::regex{".[Gg][0-9]{2}$"};

    const auto saveDirectories = std::filesystem::directory_iterator{
        GetBakDirectoryPath() / "GAMES"};

    std::vector<SaveDirectory> saveDirs{};
    for (const auto& directory : saveDirectories)
    {
        const auto dirName = directory.path().filename().string();
        Logging::LogDebug(__FUNCTION__) << "Directory: " << dirName << " matches: " << std::regex_search(dirName, dirSuffix) << std::endl;
        if (std::regex_search(dirName, dirSuffix))
        {
            saveDirs.emplace_back(
                SaveDirectory{
                    directory.path().stem().string(),
                    MakeSaveFiles(directory.path())});
        }
    }

    return saveDirs;
}

std::string LoadSaveName(FileBuffer& fb)
{
    fb.Seek(0);
    return fb.GetString(30);
}

}
