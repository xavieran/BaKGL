#include "bak/hotspot.hpp"

#include "bak/gameState.hpp"
#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"
#include "com/string.hpp"

#include "graphics/glm.hpp"

namespace BAK {

std::string_view ToString(HotspotAction ha)
{
    switch (ha)
    {
        case HotspotAction::UNKNOWN_0:   return "UNKNOWN_0";
        case HotspotAction::UNKNOWN_1:   return "UNKNOWN_1";
        case HotspotAction::DIALOG:      return "Dialog";
        case HotspotAction::EXIT:        return "Exit";
        case HotspotAction::GOTO:        return "Goto";
        case HotspotAction::BARMAID:     return "Barmaid";
        case HotspotAction::SHOP:        return "Shop";
        case HotspotAction::INN:         return "Inn";
        case HotspotAction::CONTAINER:   return "Container";
        case HotspotAction::LUTE:        return "Lute";
        case HotspotAction::REPAIR_2:   return "REPAIR_2";
        case HotspotAction::TELEPORT:    return "Teleport";
        case HotspotAction::UNKNOWN_C:   return "UNKNOWN_C";
        case HotspotAction::TEMPLE:      return "Temple";
        case HotspotAction::UNKNOWN_E:   return "UNKNOWN_E";
        case HotspotAction::CHAPTER_END: return "CHAPTER_END";
        case HotspotAction::REPAIR:      return "Repair";
        case HotspotAction::UNKNOWN_X:   return "UNKNOWN_X";

        default: std::cerr << "Unknown!! " << +static_cast<unsigned>(ha) << std::endl; throw std::runtime_error("Unknown HotspotAction");
    }
}

std::ostream& operator<<(std::ostream& os, HotspotAction ha)
{
    return os << ToString(ha);
}

Hotspot::Hotspot(
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
    std::uint16_t checkEventState)
:
    mHotspot{hotspot},
    mTopLeft{topLeft},
    mDimensions{dimensions},
    mChapterMask{chapterMask},
    mKeyword{keyword},
    mAction{action},
    mUnknown_d{unknown_d},
    mActionArg1{actionArg1},
    mActionArg2{actionArg2},
    mActionArg3{actionArg3},
    mTooltip{tooltip},
    mUnknown_1a{unknown_1a},
    mDialog{dialog},
    mCheckEventState{checkEventState}
{}

// Yes but there is more to it - e.g. Sarth
bool Hotspot::IsActive(GameState& gameState) const
{
    // ovr148:86D test mChapterMask, 0x8000???

    const auto state = CreateChoice(mDialog.mValue & 0xffff);
    const auto expectedVal = (mDialog.mValue >> 16) & 0xffff;
    Logging::LogDebug(__FUNCTION__) << "Unk2: " << mCheckEventState << " Eq: " <<
        (mCheckEventState == 1) << " GS: " << std::hex << state << " st: " << 
        gameState.GetEventState(state) << " exp: " << expectedVal << " Unk0: " << mChapterMask << "\n";
    if (mCheckEventState != 0 && !std::holds_alternative<NoChoice>(state))
    {
        return gameState.GetEventState(state) == expectedVal;
    }
    return (mChapterMask ^ 0xffff) & (1 << (gameState.GetChapter().mValue - 1));
}

bool Hotspot::EvaluateImmediately() const
{
    return (mChapterMask & 0x8000) != 0;
}

std::ostream& operator<<(std::ostream& os, const Hotspot& hs)
{
    os << "Hotspot { id: " << hs.mHotspot << " topLeft: " << hs.mTopLeft
        << " dim: " << hs.mDimensions << " kw: " << hs.mKeyword 
        << " act: " << hs.mAction << " arg1: " << std::hex << hs.mActionArg1
        << " arg2: " << hs.mActionArg2 
        << " arg3: " << hs.mActionArg3
        << " tip: " << hs.mTooltip
        << " dialog: " << hs.mDialog 
        << " chapterMask: " << hs.mChapterMask << " | " << hs.mUnknown_1a
        << " | " << hs.mCheckEventState << std::dec
        << " }";
    return os;
}

SceneHotspots::SceneHotspots(FileBuffer&& fb)
{
    const auto& logger = Logging::LogState::GetLogger("BAK::SceneHotspots");
    auto length = fb.GetUint16LE();
    logger.Debug() << "Length: " << length << std::endl;
    const auto resource = fb.GetString(6);
    mSceneTTM = ToUpper(resource + ".TTM");
    mSceneADS = ToUpper(resource + ".ADS");
    logger.Debug() << "Scene Ref: " << mSceneTTM << std::endl;

    mUnknown_6 = fb.GetArray<6>();
    logger.Debug() << "Unknown_6: " << std::hex 
        << mUnknown_6 << std::dec << "\n";
    mUnknown_c = fb.GetUint8();
    logger.Debug() << "Unknown_C: " << std::hex 
        << +mUnknown_c << std::dec << "\n";
    mTempleIndex = fb.GetUint8();
    logger.Debug() << "TempleIndex: " << std::hex 
        << +mTempleIndex << std::dec << "\n";
    mUnknown_e = fb.GetUint8();
    logger.Debug() << "Unknown_E: " << std::hex 
        << +mUnknown_e << std::dec << "\n";
    mUnknown_f = fb.GetUint8();
    logger.Debug() << "Unknown_F: " << std::hex 
        << +mUnknown_f << std::dec << "\n";
    mUnknown_10 = fb.GetUint8();
    logger.Debug() << "Unknown_10: " << std::hex
        << +mUnknown_10 << std::dec << "\n";
    mSong = fb.GetUint16LE();
    logger.Debug() << "Song : " << mSong << "\n";
    mUnknownIdx_13 = fb.GetUint16LE();
    logger.Debug() << "Unknown_13: " << std::hex
        << +mUnknownIdx_13 << std::dec << "\n";
    // For all towns, scene index1.
    mSceneIndex1 = fb.GetUint16LE();
    logger.Debug() << "Scene index1: " << mSceneIndex1 << "\n";
    const auto mUnkown_16 = fb.GetUint16LE();
    logger.Debug() << "Unknown_16:" << mUnkown_16 << "\n";
    mSceneIndex2 = fb.GetUint16LE();
    logger.Debug() << "Scene index2: " << mSceneIndex2 << "\n";
    auto numHotSpots = fb.GetUint16LE(); 
    mFlavourText = fb.GetUint32LE(); 
    logger.Debug() << "Hotspots: " << std::dec << numHotSpots << std::endl;
    logger.Debug() << "Flavour Text: " << std::hex << mFlavourText << std::endl;

    mUnknown_1f = fb.GetUint16LE();
    logger.Debug() << "Unknown_1f:" << mUnknown_1f << "\n";
    mUnknown_21 = fb.GetUint16LE();
    logger.Debug() << "Unknown_21:" << mUnknown_21 << "\n";
    mUnknown_23 = fb.GetUint16LE();
    logger.Debug() << "Unknown_23:" << mUnknown_23 << "\n";
    mUnknown_25 = fb.GetUint16LE();
    logger.Debug() << "Unknown_25:" << mUnknown_25 << "\n";

    std::vector<Hotspot> hotspots;

    for (unsigned i = 0; i < numHotSpots; i++)
    {
        auto x = fb.GetUint16LE();
        auto y = fb.GetUint16LE();
        auto w = fb.GetUint16LE();
        auto h = fb.GetUint16LE();
        logger.Debug() << "Hotspot #" << std::dec << i << std::endl;
        const auto chapterMask = fb.GetUint16LE();
        logger.Debug() << "ChapterMask: " << std::hex << chapterMask << std::dec << std::endl;
        const auto keyword = fb.GetUint16LE();
        const auto action = fb.GetUint8();
        const auto unknown_D = fb.GetUint8();
        logger.Debug() << "keyword: " << keyword << std::endl;
        logger.Debug() << "Action: " << +action << std::endl;
        logger.Debug() << "Unknown_D: " << +unknown_D << std::endl;
        const auto actionArg1 = fb.GetUint16LE();
        const auto actionArg2 = fb.GetUint16LE();
        const auto actionArg3 = fb.GetUint32LE();
        logger.Debug() << "A1: " << actionArg1 << std::hex << " A2: " << actionArg2 << " A3: " << actionArg3 << std::dec << "\n";
        std::uint32_t tooltip = fb.GetUint32LE(); 
        logger.Debug() << "RightClick: " << std::hex << tooltip << std::endl;
        const auto unknown_1a = fb.GetUint32LE();
        logger.Debug() << "Unknown_1a: " << std::hex << unknown_1a << std::dec << std::endl;
        std::uint32_t dialog = fb.GetUint32LE(); 
        logger.Debug() << "LeftClick: " << std::hex << dialog << std::endl;
        const auto checkEventState = fb.GetUint16LE();
        logger.Debug() << "CheckEventState: " << std::hex << checkEventState << std::dec << std::endl;
        //const auto unknown_24 = fb.GetUint8();
        //const auto containerAddr = fb.GetUint8();
        //logger.Debug() << "unknown_24: " << +unknown_24 << "\n";
        //logger.Debug() << "containerAddr: " << +containerAddr << "\n";

        hotspots.emplace_back(
            i,
            glm::vec<2, int>{x, y},
            glm::vec<2, int>{w, h},
            chapterMask,
            keyword,
            static_cast<HotspotAction>(action),
            unknown_D,
            actionArg1,
            actionArg2,
            actionArg3,
            KeyTarget{tooltip},
            unknown_1a,
            KeyTarget{dialog},
            checkEventState);
    }

    mHotspots = hotspots;

    for (auto& hs : hotspots)
    {
        logger.Debug() << hs << "\n";
    }

    logger.Debug() << "ADS: " << mSceneADS
        << " TTM: " << mSceneTTM
        << " " << mSceneIndex1
        << " " << mSceneIndex2 << "\n";

    auto fb2 = FileBufferFactory::Get().CreateDataBuffer(mSceneADS);
    mAdsIndices = BAK::LoadSceneIndices(fb2);
    auto fb3 = FileBufferFactory::Get().CreateDataBuffer(mSceneTTM);
    mScenes = BAK::LoadScenes(fb3);
}

const Scene& SceneHotspots::GetScene(
    unsigned adsIndex,
    const GameState& gs)
{
    const auto ttmIndex = mAdsIndices[adsIndex];
    return mScenes[ttmIndex.mSceneIndex.GetTTMIndex(gs.GetChapter())];
}

std::optional<unsigned> SceneHotspots::GetTempleNumber() const
{
    if (!(0x80 & mTempleIndex)) return std::nullopt;
    return mTempleIndex & 0x7f;
    
}
}
