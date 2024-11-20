#include "bak/file/packedFileProvider.hpp"

#include "bak/image.hpp"
#include "bak/imageStore.hpp"
#include "bak/palette.hpp"
#include "bak/screen.hpp"

#include "com/png.hpp"
extern "C" {
#include "com/getopt.h"
}

#include "com/logger.hpp"
#include "com/string.hpp"

#include <filesystem>

std::unordered_map<std::string, std::string> imageToPalMap{
    // Actors
    {"ACT001.BMX", "ACT001.PAL"},
    {"ACT001A.BMX", "ACT001.PAL"},
    {"ACT002.BMX", "ACT002.PAL"},
    {"ACT002A.BMX", "ACT002.PAL"},
    {"ACT003.BMX", "ACT003.PAL"},
    {"ACT003A.BMX", "ACT003.PAL"},
    {"ACT004.BMX", "ACT004.PAL"},
    {"ACT004A.BMX", "ACT004.PAL"},
    {"ACT005.BMX", "ACT005.PAL"},
    {"ACT005A.BMX", "ACT005.PAL"},
    {"ACT006.BMX", "ACT006.PAL"},
    {"ACT006A.BMX", "ACT006.PAL"},
    {"ACT007.BMX", "ACT007.PAL"},
    {"ACT008.BMX", "ACT008.PAL"},
    {"ACT009A.BMX", "ACT009.PAL"},
    {"ACT010.BMX", "ACT010.PAL"},
    {"ACT011.BMX", "ACT011.PAL"},
    {"ACT012A.BMX", "ACT012.PAL"},
    {"ACT013.BMX", "ACT013.PAL"},
    {"ACT014.BMX", "ACT014.PAL"},
    {"ACT015.BMX", "ACT015.PAL"},
    {"ACT016.BMX", "ACT016.PAL"},
    {"ACT017.BMX", "ACT017.PAL"},
    {"ACT018A.BMX", "ACT018.PAL"},
    {"ACT019.BMX", "ACT019.PAL"},
    {"ACT020.BMX", "ACT020.PAL"},
    {"ACT021.BMX", "ACT021.PAL"},
    {"ACT022.BMX", "ACT022.PAL"},
    {"ACT023.BMX", "ACT023.PAL"},
    {"ACT024.BMX", "ACT024.PAL"},
    {"ACT025.BMX", "ACT025.PAL"},
    {"ACT026.BMX", "ACT026.PAL"},
    {"ACT027.BMX", "ACT027.PAL"},
    {"ACT028.BMX", "ACT028.PAL"},
    {"ACT029.BMX", "ACT029.PAL"},
    {"ACT030A.BMX", "ACT030.PAL"},
    {"ACT031.BMX", "ACT031.PAL"},
    {"ACT032.BMX", "ACT032.PAL"},
    {"ACT033.BMX", "ACT033.PAL"},
    {"ACT034.BMX", "ACT034.PAL"},
    {"ACT035.BMX", "ACT035.PAL"},
    {"ACT036.BMX", "ACT036.PAL"},
    {"ACT037.BMX", "ACT037.PAL"},
    {"ACT038.BMX", "ACT038.PAL"},
    {"ACT039.BMX", "ACT039.PAL"},
    {"ACT040.BMX", "ACT040.PAL"},
    {"ACT041.BMX", "ACT041.PAL"},
    {"ACT042.BMX", "ACT042.PAL"},
    {"ACT043.BMX", "ACT043.PAL"},
    {"ACT044.BMX", "ACT044.PAL"},
    {"ACT045.BMX", "ACT045.PAL"},
    {"ACT046.BMX", "ACT046.PAL"},
    {"ACT047.BMX", "ACT047.PAL"},
    {"ACT048.BMX", "ACT048.PAL"},
    {"ACT049.BMX", "ACT049.PAL"},
    {"ACT050.BMX", "ACT050.PAL"},
    {"ACT051.BMX", "ACT051.PAL"},
    {"ACT052.BMX", "ACT052.PAL"},
    {"ACT053.BMX", "ACT053.PAL"},

    // Chapters
    {"C11A1.BMX", "C11A.PAL"},
    {"C11A2.BMX", "C11A.PAL"},
    {"C11B.BMX", "C11B.PAL"},
    {"C12A.BMX", "C12A.PAL"},
    {"C12A_BAK.BMX", "C12A.PAL"},
    {"C12A_MAG.BMX", "C12A.PAL"},
    {"C12A_PUG.BMX", "C12A.PAL"},
    {"C12B_ARC.BMX", "C12B.PAL"},
    {"C12B_GOR.BMX", "C12B.PAL"},
    {"C12B_SRL.BMX", "C12A.PAL"},
    {"C21A.BMX", "C21.PAL"},
    {"C21A_BAK.BMX", "C21.PAL"},
    {"C21B1.BMX", "C21.PAL"},
    {"C21C.BMX", "C21.PAL"},
    {"C21_MAK.BMX", "C21.PAL"},
    {"C22.BMX", "C22.PAL"},
    {"C31A_BAK.BMX", "C31.PAL"},
    {"C31A_JIM.BMX", "C31.PAL"},
    {"C31A_PYR.BMX", "C31.PAL"},
    {"C31B_BAK.BMX", "C31.PAL"},
    {"C31B_GOR.BMX", "C31.PAL"},
    {"C32A_BAK.BMX", "C32A.PAL"},
    {"C32A_WLK.BMX", "C32A.PAL"},
    {"C32B_BAK.BMX", "C32B.PAL"},
    {"C41A_BAK.BMX", "C41A.PAL"},
    {"C41A_DEL.BMX", "C41A.PAL"},
    {"C41A_DOR.BMX", "C41A.PAL"},
    {"C41A_OWD.BMX", "C41A.PAL"},
    {"C41A_OWO.BMX", "C41A.PAL"},
    {"C41B_BAK.BMX", "C41B.PAL"},
    {"C41B_DEL.BMX", "C41B.PAL"},
    {"C41B_GOR.BMX", "C41B.PAL"},
    {"C42_PNTR.BMX", "C42.PAL"},
    {"C42_WNDW.BMX", "C42.PAL"},
    {"C51A_BAK.BMX", "C51.PAL"},
    {"C51A_MOR.BMX", "C51.PAL"},
    {"C51A_PTR.BMX", "C51.PAL"},
    {"C51B_BAK.BMX", "C51.PAL"},
    {"C51B_JNL.BMX", "C51.PAL"},
    {"C52A_BAK.BMX", "C52A.PAL"},
    {"C52A_JIM.BMX", "C52A.PAL"},
    {"C52A_MOR.BMX", "C52A.PAL"},
    {"C52B_ARU.BMX", "C52B.PAL"},
    {"C52B_BAK.BMX", "C52B.PAL"},
    {"C52B_JIM.BMX", "C52B.PAL"},
    {"C61A_BAK.BMX", "C61A.PAL"},
    {"C61A_CHS.BMX", "C61A.PAL"},
    {"C61A_GAT.BMX", "C61A.PAL"},
    {"C61A_MAK.BMX", "C61A.PAL"},
    {"C61A_TLK.BMX", "C61B.PAL"},
    {"C61B_BAK.BMX", "C61B.PAL"},
    {"C61B_MAK.BMX", "C61B.PAL"},
    {"C61C_BAK.BMX", "C61C.PAL"},
    {"C61C_PUG.BMX", "C61C.PAL"},
    {"C61C_TLK.BMX", "C61C.PAL"},
    {"C61D_BAK.BMX", "C61D.PAL"},
    {"C61D_BLA.BMX", "C61D.PAL"},
    {"C61D_MAC.BMX", "C61D.PAL"},
    {"C62A.BMX", "C62A.PAL"},
    {"C62B_BG1.BMX", "C62B.PAL"},
    {"C62B_BG2.BMX", "C62B.PAL"},
    {"C62B_BOK.BMX", "C62B.PAL"},
    {"C62B_BRU.BMX", "C62B.PAL"},
    {"C62B_QUE.BMX", "C62B.PAL"},
    {"C62B_TOM.BMX", "C62B.PAL"},
    {"C71A_AR1.BMX", "C71A.PAL"},
    {"C71A_AR2.BMX", "C71A.PAL"},
    {"C71A_BG.BMX", "C71A.PAL"},
    {"C71B.BMX", "C71B.PAL"},
    {"C71B_BG.BMX", "C71B.PAL"},
    {"C71C.BMX", "C71C.PAL"},
    {"C71C_BG.BMX", "C71C.PAL"},
    {"C72A_BG.BMX", "C72A.PAL"},
    {"C72A_LEA.BMX", "C72A.PAL"},
    {"C72A_PAT.BMX", "C72A.PAL"},
    {"C72B_BG.BMX", "C72B.PAL"},
    {"C72B_HLD.BMX", "C72B.PAL"},
    {"C72B_PAT.BMX", "C72B.PAL"},
    {"C72C_BG.BMX", "C72C.PAL"},
    {"C72C_PTY.BMX", "C72C.PAL"},
    {"C81.BMX", "C81.PAL"},
    {"C82A.BMX", "C82A.PAL"},
    {"C82A_CEL.BMX", "C82A.PAL"},
    {"C82B.BMX", "C82B.PAL"},
    {"C82B_GOR.BMX", "C82B.PAL"},
    {"C82C.BMX", "C82C.PAL"},
    {"C82C_GAM.BMX", "C82C.PAL"},
    {"C91_BG.BMX", "C91.PAL"},
    {"C91_GOR.BMX", "C91_GOR.PAL"},
    {"C91_JIM.BMX", "C91_JIM.PAL"},
    {"C91_PRTY.BMX", "C91.PAL"},
    {"C91_PUG.BMX", "C91_PUG.PAL"},
    {"C92.BMX", "C92.PAL"},
    {"C93A.BMX", "C93A.PAL"},
    {"C93A_1.BMX", "C93A.PAL"},
    {"C93B.BMX", "C93B.PAL"},
    {"C93B_1.BMX", "C93B.PAL"},
    {"C93B_2.BMX", "C93B.PAL"},
    {"C93C.BMX", "C93C.PAL"},
    {"C93C_1A.BMX", "C93C.PAL"},
    {"C93C_1B.BMX", "C93C.PAL"},
    {"C93C_1C.BMX", "C93C.PAL"},
    {"C93D.BMX", "C93D.PAL"},
    {"C93D_1.BMX", "C93D.PAL"},

    // Shops
    {"SHOP1.BMX", "SHOP1.PAL"},
    {"SHOP1ARM.BMX", "SHOP1.PAL"},
    {"SHOP1BAK.BMX", "SHOP1.PAL"},
    {"SHOP2.BMX", "SHOP2.PAL"},
    {"SHOP2ARM.BMX", "SHOP2.PAL"},
    {"SHOP2BAK.BMX", "SHOP2.PAL"},
    {"SHOP3.BMX", "SHOP3.PAL"},
    {"SHOP3ARM.BMX", "SHOP3.PAL"},
    {"SHOP3BAK.BMX", "SHOP3.PAL"},
    {"SHOP4.BMX", "SHOP4.PAL"},

    // Taverns
    {"TVRN1.BMX", "TVRN1.PAL"},
    {"TVRN1BAK.BMX", "TVRN1.PAL"},
    {"TVRN1PPL.BMX", "TVRN1.PAL"},
    {"TVRN2.BMX", "TVRN2.PAL"},
    {"TVRN2BAK.BMX", "TVRN2.PAL"},
    {"TVRN2PPL.BMX", "TVRN2.PAL"},
    {"TVRN3.BMX", "TVRN3.PAL"},
    {"TVRN3BAK.BMX", "TVRN3.PAL"},
    {"TVRN3PPL.BMX", "TVRN3.PAL"},
    {"TVRN4.BMX", "TVRN4.PAL"},
    {"TVRN4BAK.BMX", "TVRN4.PAL"},
    {"TVRN4PPL.BMX", "TVRN4.PAL"},
    {"TVRN5.BMX", "TVRN5.PAL"},
    {"TVRN5BAK.BMX", "TVRN5.PAL"},
    {"TVRN5PPL.BMX", "TVRN5.PAL"},

    // Zones
    {"Z01H.BMX", "Z01.PAL"},
    {"Z01SLOT0.BMX", "Z01.PAL"},
    {"Z01SLOT1.BMX", "Z01.PAL"},
    {"Z01SLOT2.BMX", "Z01.PAL"},
    {"Z01SLOT3.BMX", "Z01.PAL"},
    {"Z01SLOT4.BMX", "Z01.PAL"},
    {"Z02H.BMX", "Z02.PAL"},
    {"Z02SLOT0.BMX", "Z02.PAL"},
    {"Z02SLOT1.BMX", "Z02.PAL"},
    {"Z02SLOT2.BMX", "Z02.PAL"},
    {"Z02SLOT3.BMX", "Z02.PAL"},
    {"Z02SLOT4.BMX", "Z02.PAL"},
    {"Z03H.BMX", "Z03.PAL"},
    {"Z03SLOT0.BMX", "Z03.PAL"},
    {"Z03SLOT1.BMX", "Z03.PAL"},
    {"Z03SLOT2.BMX", "Z03.PAL"},
    {"Z03SLOT3.BMX", "Z03.PAL"},
    {"Z03SLOT4.BMX", "Z03.PAL"},
    {"Z04H.BMX", "Z04.PAL"},
    {"Z04SLOT0.BMX", "Z04.PAL"},
    {"Z04SLOT1.BMX", "Z04.PAL"},
    {"Z04SLOT2.BMX", "Z04.PAL"},
    {"Z04SLOT3.BMX", "Z04.PAL"},
    {"Z04SLOT4.BMX", "Z04.PAL"},
    {"Z05H.BMX", "Z05.PAL"},
    {"Z05SLOT0.BMX", "Z05.PAL"},
    {"Z05SLOT1.BMX", "Z05.PAL"},
    {"Z05SLOT2.BMX", "Z05.PAL"},
    {"Z05SLOT3.BMX", "Z05.PAL"},
    {"Z05SLOT4.BMX", "Z05.PAL"},
    {"Z06H.BMX", "Z06.PAL"},
    {"Z06SLOT0.BMX", "Z06.PAL"},
    {"Z06SLOT1.BMX", "Z06.PAL"},
    {"Z06SLOT2.BMX", "Z06.PAL"},
    {"Z06SLOT3.BMX", "Z06.PAL"},
    {"Z06SLOT4.BMX", "Z06.PAL"},
    {"Z07H.BMX", "Z07.PAL"},
    {"Z07SLOT0.BMX", "Z07.PAL"},
    {"Z07SLOT1.BMX", "Z07.PAL"},
    {"Z07SLOT2.BMX", "Z07.PAL"},
    {"Z07SLOT3.BMX", "Z07.PAL"},
    {"Z07SLOT4.BMX", "Z07.PAL"},
    {"Z08SLOT0.BMX", "Z08.PAL"},
    {"Z08SLOT1.BMX", "Z08.PAL"},
    {"Z08SLOT2.BMX", "Z08.PAL"},
    {"Z08SLOT3.BMX", "Z08.PAL"},
    {"Z08SLOT4.BMX", "Z08.PAL"},
    {"Z09SLOT0.BMX", "Z09.PAL"},
    {"Z09SLOT1.BMX", "Z09.PAL"},
    {"Z09SLOT2.BMX", "Z09.PAL"},
    {"Z09SLOT3.BMX", "Z09.PAL"},
    {"Z09SLOT4.BMX", "Z09.PAL"},
    {"Z10SLOT0.BMX", "Z10.PAL"},
    {"Z10SLOT1.BMX", "Z10.PAL"},
    {"Z10SLOT2.BMX", "Z10.PAL"},
    {"Z10SLOT3.BMX", "Z10.PAL"},
    {"Z10SLOT4.BMX", "Z10.PAL"},
    {"Z10SLOT5.BMX", "Z10.PAL"},
    {"Z11SLOT0.BMX", "Z11.PAL"},
    {"Z11SLOT1.BMX", "Z11.PAL"},
    {"Z11SLOT2.BMX", "Z11.PAL"},
    {"Z11SLOT3.BMX", "Z11.PAL"},
    {"Z11SLOT4.BMX", "Z11.PAL"},
    {"Z11SLOT5.BMX", "Z11.PAL"},
    {"Z11SLOT6.BMX", "Z11.PAL"},
    {"Z12SLOT0.BMX", "Z12.PAL"},
    {"Z12SLOT1.BMX", "Z12.PAL"},
    {"Z12SLOT2.BMX", "Z12.PAL"},
    {"Z12SLOT3.BMX", "Z12.PAL"},
    {"Z12SLOT4.BMX", "Z12.PAL"},
    {"Z12SLOT5.BMX", "Z12.PAL"},
    {"Z12SLOT6.BMX", "Z12.PAL"},

    // Icons
    {"FMAP_ICN.BMX", "FULLMAP.PAL"},
};

struct Options
{
    std::string mResourceFile;
    std::string mResourceIndexFile;
    std::string mOutputPath;
};

void show_help()
{
    std::cout << "Extracts BaK data files into the specified directory. All arguments are mandatory\n";
    std::cout << "\t --resource,-r :: the krondor.001 file\n";
    std::cout << "\t --index,-i :: the krondor.rmf file\n";
    std::cout << "\t --output,-o :: the directory to write the unpacked data files\n";
    exit(0);
}

Options Parse(int argc, char** argv)
{
    Options values{};

    struct option options[] = {
        {"help", no_argument, 0, 'h'},
        {"resource", no_argument, 0, 'r'},
        {"index", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'}
    };
    int optionIndex = 0;
    int opt;
    if (argc == 1)
    {
        show_help();
    }
    while ((opt = getopt_long(argc, argv, "hr:i:o:", options, &optionIndex)) != -1)
    {   
        if (opt == 'h')
        {
            show_help();
        }
        else if (opt == 'r')
        {
            if (optarg == nullptr)
            {
                std::cerr << "must provide a resource file to -r argument" << std::endl;
                exit(1);
            }
            values.mResourceFile = std::string{optarg};
        }
        else if (opt == 'i')
        {
            if (optarg == nullptr)
            {
                std::cerr << "must provide a resource index file to -i argument" << std::endl;
                exit(1);
            }
            values.mResourceIndexFile = std::string{optarg};
        }
        else if (opt == 'o')
        {
            if (optarg == nullptr)
            {
                std::cerr << "must provide an output path to -o argument" << std::endl;
                exit(1);
            }
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
    PNGImage pngImage{image.GetWidth(), image.GetHeight(), {}};
    for (auto index : image.GetVector())
    {
        auto color = palette.GetColors8()[index];
        pngImage.mPixels.emplace_back(color[0], color[1], color[2], color[3]);
    }
    WritePNG(filePath.string().c_str(), pngImage);
}

void WriteImages(
    std::filesystem::path outputPath,
    std::string fileName,
    const std::vector<BAK::Image>& images,
    const BAK::Palette& palette)
{
    const auto outputName = fileName + ".PNG";
    if (images.size() == 1)
    {
        WriteImage(outputPath / outputName, images.back(), palette);
    }
    else
    {
        const auto dirName = fileName + ".BMX";
        std::filesystem::create_directory(outputPath / dirName);
        for (unsigned i = 0; i < images.size(); i++)
        {
            std::stringstream name{};
            name << i << ".PNG";
            WriteImage(outputPath / dirName / name.str(), images[i], palette);
        }
    }
}

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Info);
    Logging::LogState::SetLogTime(false);
    Logging::LogState::SetLogColor(true);
    
    const auto options = Parse(argc, argv);
    if (!std::filesystem::exists(options.mResourceFile))
    {
        logger.Error() << "Resource file does not exist at path: " << options.mResourceFile << "\n";
        show_help();
    }
    if (!std::filesystem::exists(options.mResourceIndexFile))
    {
        logger.Error() << "Resource index file does not exist at path: " << options.mResourceIndexFile << "\n";
        show_help();
    }

    if (options.mOutputPath.empty())
    {
        logger.Error() << "Must provide an output directory to write files to ('-o' option)" << std::endl;
        show_help();
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
        
        if (imageToPalMap.contains(fname)) {
            palName = imageToPalMap.at(fname);
        }
        else if (palFiles.contains(name + ".PAL"))
        {
            palName = name + ".PAL";
        }
        auto palette = palFiles.at(palName);
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
        WriteImage(output / (name + ".PNG"), image, palette);
    }
}
