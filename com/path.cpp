#include "com/path.hpp"

#include <filesystem>

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

std::string GetBakDirectory()
{
    const auto path = std::filesystem::path{GetHomeDirectory()} / "bak";
    return path.string();
}
