#pragma once

#include "bak/dialogTarget.hpp"

namespace BAK::Encounter {

/*
3F 00 00 00
01 04 00 0F FE 1C 00 00 00
01 27 00 F5 32 29 00 00 00
*/

class Block
{
public:
    KeyTarget mDialog;
};

std::ostream& operator<<(std::ostream& os, const Block&);

class BlockFactory
{
public:
    static constexpr auto sFilename = "DEF_BLOC.DAT";

    BlockFactory();

    const Block& Get(unsigned i) const;

private:
    void Load();
    std::vector<Block> mBlocks;
};

}
