#include "bak/file/packedFileProvider.hpp"
#include "bak/imageStore.hpp"
#include "bak/palette.hpp"
#include "bak/screen.hpp"

#include "com/bmpWriter.hpp"
extern "C" {
#include "com/getopt.h"
}

#include "com/logger.hpp"
#include "com/string.hpp"

#include <filesystem>

struct Options
{
    std::string mResourceFile;
    std::string mResourceIndexFile;
    std::string mOutputPath;
};

Options Parse(int argc, char** argv)
{
    Options values{};

    struct option options[] = {
        {"help", no_argument,       0, 'h'},
        {"resource", no_argument, 0, 'r'},
        {"index", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'}
    };
    int optionIndex = 0;
    int opt;
    while ((opt = getopt_long(argc, argv, "hr:i:o:", options, &optionIndex)) != -1)
    {   
        if (opt == 'h')
        {
            std::cout << "Extracts BaK data files into the specified directory. All arguments are mandatory\n";
            std::cout << "\t --resource,-r :: the krondor.001 file\n";
            std::cout << "\t --index,-i :: the krondor.rmf file\n";
            std::cout << "\t --output,-o :: the directory to write the unpacked data files\n";
            exit(0);
        }
        else if (opt == 'r')
        {
            values.mResourceFile = std::string{optarg};
        }
        else if (opt == 'i')
        {
            values.mResourceIndexFile = std::string{optarg};
        }
        else if (opt == 'o')
        {
            values.mOutputPath = std::string{optarg};
        }
    }

    return values;
}

void WriteImage(
    std::filesystem::path filePath,
    const BAK::Image& image,
    const BAK::Palette& palette)
{
    auto fout = std::ofstream{
        filePath,
        std::ios::binary | std::ios::out};
    WriteBMP(fout, image.GetWidth(), image.GetHeight(), image.GetVector(), palette.GetColors8());
}

void WriteImages(
    std::filesystem::path outputPath,
    std::string fileName,
    const std::vector<BAK::Image>& images,
    const BAK::Palette& palette)
{
    const auto outputName = fileName + ".BMP";
    if (images.size() == 1)
    {
        WriteImage(outputPath / outputName, images.back(), palette);
    }
    else
    {
        std::filesystem::create_directory(outputPath / outputName);
        for (unsigned i = 0; i < images.size(); i++)
        {
            std::stringstream name{};
            name << i << ".BMP";
            WriteImage(outputPath / outputName / name.str(), images[i], palette);
        }
    }
}

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Info);
    Logging::LogState::SetLogTime(false);
    
    const auto options = Parse(argc, argv);
    if (!std::filesystem::exists(options.mResourceFile))
    {
        logger.Error() << "Resource file does not exist at path: " << options.mResourceFile << "\n";
        exit(1);
    }
    if (!std::filesystem::exists(options.mResourceIndexFile))
    {
        logger.Error() << "Resource index file does not exist at path: " << options.mResourceIndexFile << "\n";
        exit(1);
    }

    logger.Info() << "Extracting resources to:" << options.mOutputPath << "\n";
    auto output = std::filesystem::path(options.mOutputPath);
    if (!std::filesystem::exists(output))
    {
        logger.Info() << "Creating output path: " << options.mOutputPath << "\n";
        std::filesystem::create_directory(output);
    }

    auto packed = BAK::File::PackedFileDataProvider{options.mResourceFile, options.mResourceIndexFile};
    auto& cache = packed.GetCache();

    std::unordered_map<std::string, std::vector<BAK::Image>> bmxFiles{};
    std::unordered_map<std::string, BAK::Palette> palFiles{};
    std::unordered_map<std::string, BAK::Image> scxFiles{};

    for (auto& [file, buffer] : cache)
    {
        if (file.find(".BMX") != std::string::npos)
        {
            bmxFiles[file] = BAK::LoadImages(buffer);
        }
        if (file.find(".PAL") != std::string::npos)
        {
            palFiles.emplace(file, BAK::Palette(buffer));
        }
        if (file.find(".SCX") != std::string::npos)
        {
            scxFiles.emplace(file, BAK::LoadScreenResource(buffer));
        }
    }

    for (const auto& [fname, images] : bmxFiles)
    {
        auto name = SplitString(".", fname)[0];
        std::string palName = "OPTIONS.PAL";
        auto palette = palFiles.at(palName);
        if (palFiles.contains(name + ".PAL"))
        {
            palName = name + ".PAL";
            palette = palFiles.at(palName);
        }
        logger.Info() << "Writing out BMX (" << images.size() << " sub images) " << fname << " with palette: " << palName << "\n";
        WriteImages(output, name, images, palette);
    }

    for (const auto& [fname, image] : scxFiles)
    {
        auto name = SplitString(".", fname)[0];
        std::string palName = "OPTIONS.PAL";
        auto palette = palFiles.at(palName);
        if (palFiles.contains(name + ".PAL"))
        {
            palName = name + ".PAL";
            palette = palFiles.at(palName);
        }
        logger.Info() << "Writing out SCX " << fname << " with palette: " << palName << "\n";
        WriteImage(output / (name + ".BMP"), image, palette);
    }
}
