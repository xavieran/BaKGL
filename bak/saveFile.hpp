#pragma once

#include "bak/save.hpp"

#include "com/logger.hpp"
#include "com/path.hpp"

#include <filesystem>
#include <regex>

namespace BAK {

class SaveFile
{
public:
    unsigned mIndex;
    std::string mName;
    std::string mPath;
};

class SaveDirectory
{
public:
    unsigned mIndex;
    std::string mName;
    std::vector<SaveFile> mSaves;
};


std::string LoadSaveName(FileBuffer& fb);
std::vector<SaveDirectory> MakeSaveDirectories();

}
