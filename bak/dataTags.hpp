#pragma once

#include <cstdint>

namespace BAK {

enum class DataTag : std::uint32_t {
    ADS = 0x3a534441,
    APP = 0x3a505041,
    BIN = 0x3a4e4942,
    BMP = 0x3a504d42,
    DAT = 0x3a544144,
    FNT = 0x3a544e46,
    GID = 0x3a444947,
    INF = 0x3a464e49,
    MAP = 0x3a50414d,
    PAG = 0x3a474150,
    PAL = 0x3a4c4150,
    RES = 0x3a534552,
    SCR = 0x3a524353,
    SND = 0x3a444e53,
    TAG = 0x3a474154,
    TT3 = 0x3a335454,
    TTI = 0x3a495454,
    VER = 0x3a524556,
    VGA = 0x3a414756
};

}
