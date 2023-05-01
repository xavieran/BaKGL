#pragma once

#include "bak/dialog.hpp"
#include "bak/resourceNames.hpp"
#include "bak/scene.hpp"
#include "bak/types.hpp"

#include "com/logger.hpp"

#include "bak/fileBufferFactory.hpp"

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
    TELEPORT  = 0xb,
    TEMPLE    = 0xd,
    NOT_SURE  = 0xf,
    REPAIR    = 0x10
};

std::ostream& operator<<(std::ostream&, HotspotAction);

struct Hotspot
{
    Hotspot(
        std::uint16_t hotspot,
        glm::vec<2, int> topLeft,
        glm::vec<2, int> dimensions,
        std::uint16_t keyword,
        HotspotAction action,
        std::uint16_t actionArg1,
        std::uint16_t actionArg2,
        std::uint32_t actionArg3,
        KeyTarget tooltip,
        KeyTarget dialog,
        std::uint16_t unknown0,
        std::uint32_t unknown1,
        std::uint16_t unknown2)
    :
        mHotspot{hotspot},
        mTopLeft{topLeft},
        mDimensions{dimensions},
        mKeyword{keyword},
        mAction{action},
        mActionArg1{actionArg1},
        mActionArg2{actionArg2},
        mActionArg3{actionArg3},
        mTooltip{tooltip},
        mDialog{dialog},
        mUnknown0{unknown0},
        mUnknown1{unknown1},
        mUnknown2{unknown2}
    {}

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

    // Yes but there is more to it - e.g. Sarth
    bool IsActive(GameState& gameState) const
    {
        // ovr148:86D test mUnknown0, 0x8000???

        const auto state = mDialog.mValue & 0xffff;
        const auto expectedVal = (mDialog.mValue >> 16) & 0xffff;
        Logging::LogDebug(__FUNCTION__) << "Unk2: " << mUnknown2 << " Eq: " <<
            (mUnknown2 == 1) << " GS: " << std::hex << state << " st: " << 
            gameState.GetEventState(state) << " exp: " << expectedVal << " Unk0: " << mUnknown0 << "\n";
        if (mUnknown2 != 0 && state != 0)
        {
            return gameState.GetEventState(state) == expectedVal;
        }
        return (mUnknown0 ^ 0xffff) & (1 << (gameState.GetChapter().mValue - 1));
    }
    
    bool EvaluateImmediately() const
    {
        return (mUnknown0 & 0x8000) != 0;
    }

    std::uint16_t mUnknown0;
    std::uint32_t mUnknown1;
    std::uint16_t mUnknown2;
};

std::ostream& operator<<(std::ostream&, const Hotspot&);

// Loaded from a GDS File
class SceneHotspots
{
public:
    explicit SceneHotspots(FileBuffer&&);

    std::string mSceneTTM;
    std::string mSceneADS;

    SongIndex mSong;

    AdsSceneIndex mSceneIndex1;
    AdsSceneIndex mSceneIndex2;

    std::uint32_t mFlavourText;
    // This seems only to be used for temples??
    std::uint8_t mTempleIndex;

    std::vector<Hotspot> mHotspots;
    std::unordered_map<unsigned, SceneIndex> mAdsIndices;
    std::unordered_map<unsigned, Scene> mScenes;

    const Scene& GetScene(unsigned adsIndex, const GameState& gs);
};

}
