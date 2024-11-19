#include "bak/file/packedFileProvider.hpp"

extern "C" {
#include "com/getopt.h"
}

#include "com/logger.hpp"

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
    unsigned i = 0;
    for (auto& [file, buffer] : packed.GetCache())
    {
        i++;
        auto savePath = std::filesystem::path(output) / file;
        logger.Info() << "Writing " << buffer.GetSize() << " bytes to: " << savePath << "\n";
        auto fout = std::ofstream{
            std::filesystem::path(output) / file,
            std::ios::binary | std::ios::out};
        buffer.Save(fout);
    }
    logger.Info() << "Completed extraction. Wrote " << i << " files.\n";
}
