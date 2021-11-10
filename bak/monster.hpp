#pragma once

#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "xbak/Exception.h"
#include "xbak/FileBuffer.h"

namespace BAK {

class MonsterNames
{
public:
    MonsterNames();

    struct Monster
    {
        std::string mPrefix;
        std::uint8_t mUnknown0;
        std::uint8_t mUnknown1;
        std::uint8_t mUnknown2;
        std::uint8_t mUnknown3;
    };

private:
    std::vector<std::string> mMonsterNames;
    std::vector<Monster> mMonsterPrefixes;
};

}
