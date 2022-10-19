#pragma once

#include <filesystem>
#include <string>

std::string GetHomeDirectory();
std::filesystem::path GetBakDirectoryPath();
std::string GetBakDirectory();
