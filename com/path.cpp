#include "com/path.hpp"

std::string GetHomeDirectory()
{
#ifdef _MSC_VER
    constexpr auto homeVar = "APPDATA";
#else
    constexpr auto homeVar = "HOME";
#endif 
    auto* home = getenv(homeVar);

    if (home == nullptr)
        throw std::runtime_error("No " + std::string{homeVar} + " environment variable set.");

    return std::string{home};
}

std::filesystem::path GetBakDirectoryPath()
{
    return std::filesystem::path{GetHomeDirectory()} / "bak";
}

std::string GetBakDirectory()
{
    return GetBakDirectoryPath().string();
}
