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
    {
        throw std::runtime_error("No " + std::string{homeVar} + " environment variable set.");
    }

    return std::string{home};
}

Paths& Paths::Get()
{
    static Paths paths{};
    return paths;
}

std::filesystem::path Paths::GetBakDirectoryPath() const
{
    return mBakDirectoryPath;
}

std::string Paths::GetBakDirectory() const
{
    return GetBakDirectoryPath().string();
}

void Paths::SetBakDirectory(std::string path)
{
    mBakDirectoryPath = path;
}

Paths::Paths()
:
    mBakDirectoryPath{std::filesystem::path{GetHomeDirectory()} / "bak"}
{}
