#pragma once

#include "bak/constants.hpp"
#include "bak/dialog.hpp"
#include "bak/gameState.hpp"
#include "bak/resourceNames.hpp"
#include "bak/scene.hpp"
#include "bak/types.hpp"

#include "com/logger.hpp"
#include "com/string.hpp"

#include "xbak/Exception.h"
#include "xbak/FileBuffer.h"

#include <glm/glm.hpp>

#include <string_view>

namespace BAK {

enum class HotspotAction
{
    DIALOG    = 2,
    EXIT      = 3,
    GOTO      = 4,
    BARMAID   = 5,
    SHOP      = 6,
    INN       = 7,
    CONTAINER = 8,
    LUTE      = 9,
    TELEPORT  = 11,
    TEMPLE    = 13,
    NOT_SURE  = 15,
    REPAIR    = 16,
};

std::ostream& operator<<(std::ostream&, HotspotAction);

struct Hotspot
{
    std::uint16_t mHotspot;
    glm::vec<2, int> mTopLeft;
    glm::vec<2, int> mDimensions;
    std::uint16_t mKeyword;
    HotspotAction mAction;
    std::uint16_t mActionArg1;
    std::uint16_t mActionArg2;
    std::uint32_t mActionArg3;
    KeyTarget mTooltip;
    KeyTarget mDialog;
};

std::ostream& operator<<(std::ostream&, const Hotspot&);

// Loaded from a GDS File
class SceneHotspots
{
public:
    SceneHotspots(FileBuffer&&);

    std::string mSceneTTM;
    std::string mSceneADS;

    SongIndex mSong;

    AdsSceneIndex mSceneIndex1;
    AdsSceneIndex mSceneIndex2;

    std::uint32_t mFlavourText;

    std::vector<Hotspot> mHotspots;
    std::unordered_map<unsigned, SceneIndex> mAdsIndices;
    std::unordered_map<unsigned, Scene> mScenes;

    const Scene& GetScene(unsigned adsIndex, const GameState& gs);
};

}
