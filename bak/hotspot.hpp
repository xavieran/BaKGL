#pragma once

#include "bak/dialogTarget.hpp"
#include "bak/scene.hpp"
#include "bak/types.hpp"

#include <glm/glm.hpp>

#include <array>

namespace BAK {

class GameState;

enum class HotspotAction
{
    UNKNOWN_0   = 0,
    UNKNOWN_1   = 1,
    DIALOG      = 2,
    EXIT        = 3,
    GOTO        = 4,
    BARMAID     = 5,
    SHOP        = 6,
    INN         = 7,
    CONTAINER   = 8,
    LUTE        = 9,
    REPAIR_2   = 0xa,
    TELEPORT    = 0xb,
    UNKNOWN_C   = 0xc,
    TEMPLE      = 0xd,
    UNKNOWN_E   = 0xe,
    CHAPTER_END = 0xf,
    REPAIR      = 0x10,
    UNKNOWN_X   = 0x100
};

std::ostream& operator<<(std::ostream&, HotspotAction);

struct Hotspot
{
    Hotspot(
        std::uint16_t hotspot,
        glm::vec<2, int> topLeft,
        glm::vec<2, int> dimensions,
        std::uint16_t chapterMask,
        std::uint16_t keyword,
        HotspotAction action,
        std::uint8_t unknown_d,
        std::uint16_t actionArg1,
        std::uint16_t actionArg2,
        std::uint32_t actionArg3,
        KeyTarget tooltip,
        std::uint32_t unknown_1a,
        KeyTarget dialog,
        std::uint16_t checkEventState);

    // Yes but there is more to it - e.g. Sarth
    bool IsActive(GameState& gameState) const;
    bool EvaluateImmediately() const;

    std::uint16_t mHotspot{};
    glm::vec<2, int> mTopLeft{};
    glm::vec<2, int> mDimensions{};
    std::uint16_t mChapterMask{};
    std::uint16_t mKeyword{};
    HotspotAction mAction{};
    std::uint8_t mUnknown_d{};
    std::uint16_t mActionArg1{};
    std::uint16_t mActionArg2{};
    std::uint32_t mActionArg3{};
    KeyTarget mTooltip{};
    std::uint32_t mUnknown_1a{};
    KeyTarget mDialog{};
    std::uint16_t mCheckEventState{};
};

std::ostream& operator<<(std::ostream&, const Hotspot&);

// Loaded from a GDS File
class SceneHotspots
{
public:
    explicit SceneHotspots(FileBuffer&&);

    std::string mSceneTTM{};
    std::string mSceneADS{};

    std::array<std::uint8_t, 6> mUnknown_6{};
    std::uint8_t mUnknown_c{};
    std::uint8_t mTempleIndex{};
    std::uint8_t mUnknown_e{};
    std::uint8_t mUnknown_f{};
    std::uint8_t mUnknown_10{};
    SongIndex mSong{};
    std::uint16_t mUnknownIdx_13{};
    AdsSceneIndex mSceneIndex1{};
    std::uint16_t mUnknown_16{};
    AdsSceneIndex mSceneIndex2{};
    std::uint16_t mNumHotspots{};
    std::uint32_t mFlavourText{};
    std::uint16_t mUnknown_1f{};
    std::uint16_t mUnknown_21{};
    std::uint16_t mUnknown_23{};
    std::uint16_t mUnknown_25{};

    std::vector<Hotspot> mHotspots{};
    std::unordered_map<unsigned, SceneIndex> mAdsIndices{};
    std::unordered_map<unsigned, Scene> mScenes{};

    const Scene& GetScene(unsigned adsIndex, const GameState& gs);
    std::optional<unsigned> GetTempleNumber() const;
};

}
