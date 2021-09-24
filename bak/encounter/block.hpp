#pragma once

#include "bak/dialogTarget.hpp"

namespace BAK {

/*
3F 00 00 00
01 04 00 0F FE 1C 00 00 00
01 27 00 F5 32 29 00 00 00
01 20 00 4E FE 1C 00 00 00
01 80 00 26 FE 1C 00 00 00
00 F7 03 F5 32 29 00 00 00
00 D8 03 F6 32 29 00 00 00
01 06 00 E1 C6 2D 00 00 00
01 27 00 5B 0D 03 00 00 00
00 D8 03 F6 32 29 00 00 00
01 08 00 E6 91 21 00 00 00
00 D8 03 F6 32 29 00 00 00
00 98 03 F6 32 29 00 00 00
01 27 00 A9 86 01 00 00 00
01 27 00 5C 0D 03 00 00 00
00 D8 03 F6 32 29 00 00 00
01 27 00 FA 93 04 00 00 00
00 D8 03 F6 32 29 00 00 00
00 D8 03 F6 32 29 00 00 00
01 20 00 E5 91 21 00 00 00
01 10 00 2E A1 07 00 00 00 01 07 00 0E 33 29 00 00 00 00 EF 03 EF 32 29 00
00 00 00 98 03 F6 32 29 00 00 00 01 40 00 48 FE 1C 00
00 00 00 D8 03 F6 32 29 00 00 00 01 07 00 0E 33 29 00
00 00 01 40 00 1E FE 1C 00 00 00 01 40 00 48 FE 1C 00
00 00 01 27 00 73 AE 0A 00 00 00 00 D8 03 F6 32 29 00
00 00 01 40 00 48 FE 1C 00 00 00 01 27 00 74 AE 0A 00
00 00 01 02 00 1B 33 29 00 00 00 01 08 00 0F 33 29 00
00 00 01 08 00 73 18 23 00 00 00 01 20 00 D5 5C 15 00
00 00 01 04 00 0F FE 1C 00 00 00 01 08 00 73 18 23 00
00 00 01 01 00 0D 33 29 00 00 00 01 20 00 C9 91 21 00
00 00 01 08 00 D3 91 21 00 00 00 01 10 00 D8 91 21 00
00 00 01 08 00 73 18 23 00 00 00 01 08 00 E6 91 21 00
00 00 01 04 00 1C 33 29 00 00 00 01 20 00 20 FE 1C 00
00 00 01 02 00 DA C6 2D 00 00 00 01 04 00 1C 33 29 00
00 00 01 10 00 F4 91 21 00 00 00 01 08 00 D8 C6 2D 00
00 00 01 08 00 73 18 23 00 00 00 01 20 00 D5 5C 15 00
00 00 01 20 00 D5 5C 15 00 00 00 01 04 00 CB C6 2D 00
00 00 00 08 00 E6 27 09 00 00 00 01 24 00 E8 91 21 00
00 00 01 00 01 21 40 2C 00 00 00 01 06 00 E7 91 21 00
00 00 01 27 00 E0 91 21 00 00 00 01 10 00 D7 91 21 00
00 00 01 10 00 D7 91 21 00 00 00 01 10 00 CE 91 21 00
00 00 01 08 00 DF C6 2D 00 00 00
*/

class Block
{
public:
    KeyTarget mDialog;
};

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