#include "bak/saveFile.hpp"

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

std::vector<SaveFile> MakeSaveFiles(std::filesystem::path saveDir)
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

std::vector<SaveDirectory> MakeSaveDirectories()
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

std::string LoadSaveName(FileBuffer& fb)
{
    fb.Seek(0);
    return fb.GetString(30);
}

}
