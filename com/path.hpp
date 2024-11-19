#pragma once

#include <filesystem>
#include <string>

std::string GetHomeDirectory();
class Paths
{
public:
    static Paths& Get();

    std::filesystem::path GetBakDirectoryPath() const;
    std::string GetBakDirectory() const;

    void SetBakDirectory(std::string path);
private:
    Paths();

    std::filesystem::path mBakDirectoryPath;
};


