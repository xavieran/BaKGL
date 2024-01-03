#pragma once

#include "graphics/glm.hpp"

#include <vector>

namespace BAK {

enum class TrapElementType
{
    RedCrystal = 0x7,
    GreenCrystal = 0x8,
    SolidDiamond = 0x9,
    TransparentDiamond = 0xA,
    Unknown_C3 = 0xc3,
    Unknown_FFEE = 0xffee,
    Character0 = 0xffef,
    Character1 = 0xfff0,
    Character2 = 0xfff1,
    BlasterDown = 0xfff3,
    BlasterUp = 0xfff4,
    BlasterRight = 0xfff5,
    BlasterLeft = 0xfff6,
    Exit = 0xfffa
};

class TrapElement
{
public:
    TrapElementType mElement;
    glm::uvec2 mPosition;
};

class Trap
{
public:
    unsigned mIndex;
    std::vector<TrapElement> mElements;
};

std::ostream& operator<<(std::ostream& os, const Trap& trap);

std::vector<Trap> LoadTraps();

}
