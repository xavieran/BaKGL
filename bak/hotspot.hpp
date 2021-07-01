#pragma once

#include "bak/constants.hpp"
#include "bak/dialog.hpp"
#include "bak/resourceNames.hpp"

#include "com/logger.hpp"
#include "com/string.hpp"

#include "xbak/Exception.h"
#include "xbak/FileBuffer.h"

#include <glm/glm.hpp>

#include <string_view>

namespace BAK {

enum class HotspotAction
{
    DIALOG = 2,
    EXIT = 3,
    GOTO = 4,
    BARMAID = 5,
    SHOP = 6,
    INN = 7,
    CONTAINER = 8,
    LUTE = 9,
    TELEPORT = 11,
    TEMPLE = 13,
    NOT_SURE = 15,
    REPAIR = 16,
};

std::ostream& operator<<(std::ostream&, HotspotAction);

struct HotspotRef
{
    std::uint8_t mGdsNumber;
    char mGdsChar;

    bool operator==(const auto& rhs)
    {
        return mGdsNumber == rhs.mGdsNumber
            && mGdsChar == rhs.mGdsChar;
    }

    bool operator!=(const auto& rhs)
    {
        return !(*this == rhs);
    }

    std::string ToString() const;
    std::string ToFilename() const;
};

std::ostream& operator<<(std::ostream&, const HotspotRef&);

struct Hotspot
{
    std::uint16_t mHotspot;
    glm::vec<2, int> mTopLeft;
    glm::vec<2, int> mDimensions;
    std::uint16_t mKeyword;
    HotspotAction mAction;
    std::uint32_t mActionArg1;
    std::uint32_t mActionArg2;
    KeyTarget mTooltip;
    KeyTarget mDialog;
};

std::ostream& operator<<(std::ostream&, const Hotspot&);

// Loaded from a GDS File
class SceneHotspots
{
public:
    std::string mSceneTTM;
    std::string mSceneADS;

    std::uint16_t mSceneIndex1;
    std::uint16_t mSceneIndex2;

    std::uint32_t mFlavourText;

    std::vector<Hotspot> mHotspots;

    void Load(FileBuffer&);
};

}
