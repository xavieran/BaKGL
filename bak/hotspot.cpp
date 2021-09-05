#include "bak/hotspot.hpp"

#include "graphics/glm.hpp"

namespace BAK {

std::string_view ToString(HotspotAction ha)
{
    switch (ha)
    {
        case HotspotAction::DIALOG:    return "Dialog";
        case HotspotAction::EXIT:      return "Exit";
        case HotspotAction::GOTO:      return "Goto";
        case HotspotAction::BARMAID:   return "Barmaid";
        case HotspotAction::SHOP:      return "Shop";
        case HotspotAction::INN:       return "Inn";
        case HotspotAction::CONTAINER: return "Container";
        case HotspotAction::LUTE:      return "Lute";
        case HotspotAction::TELEPORT:  return "Teleport";
        case HotspotAction::TEMPLE:    return "Temple";
        case HotspotAction::NOT_SURE:  return "Not Sure";
        case HotspotAction::REPAIR:    return "Repair";

        default: throw std::runtime_error("Unknown HotspotAction");
    }
}

std::ostream& operator<<(std::ostream& os, HotspotAction ha)
{
    return os << ToString(ha);
}

std::ostream& operator<<(std::ostream& os, const Hotspot& hs)
{
    os << "Hotspot { id: " << hs.mHotspot << " topLeft: " << hs.mTopLeft
        << " dim: " << hs.mDimensions << " kw: " << hs.mKeyword 
        << " act: " << hs.mAction << " arg1: " << std::hex << hs.mActionArg1
        << " arg2: " << hs.mActionArg2 
        << " arg3: " << hs.mActionArg3
        << " tip: " << hs.mTooltip
        << " dialog: " << hs.mDialog << std::dec
        << " }";
    return os;
}

std::string HotspotRef::ToString() const
{
    std::stringstream ss{};
    ss << std::dec << +mGdsNumber << mGdsChar;
    return ss.str();
}

std::string HotspotRef::ToFilename() const
{
    return "GDS" + ToString() + ".DAT";
}

std::ostream& operator<<(std::ostream& os, const HotspotRef& hr)
{
    os << hr.ToString();
    return os;
}

SceneHotspots::SceneHotspots(FileBuffer&& fb)
{
    const auto& logger = Logging::LogState::GetLogger("BAK::SceneHotspots");
    auto length = fb.GetUint16LE();
    logger.Spam() << "Length: " << length << std::endl;
    const auto resource = fb.GetString(6);
    mSceneTTM = ToUpper(resource + ".TTM");
    mSceneADS = ToUpper(resource + ".ADS");
    logger.Spam() << "Scene Ref: " << mSceneTTM << std::endl;

    const auto unknownData = fb.GetArray<11>();
    logger.Spam() << "Unknown data: " << std::hex 
        << unknownData << std::dec << "\n";
    const auto song = fb.GetUint16LE();
    logger.Spam() << "Song: " << song << "\n";
    const auto unknownIndex = fb.GetUint16LE();
    logger.Spam() << "UnknownIndex: " << unknownIndex << "\n";
    // For all towns, scene index1.
    mSceneIndex1 = fb.GetUint16LE();
    logger.Spam() << "Scene index1: " << mSceneIndex1 << "\n";
    const auto unknown16 = fb.GetUint16LE();
    logger.Spam() << "Unknown 16:" << unknown16 << "\n";
    mSceneIndex2 = fb.GetUint16LE();
    logger.Spam() << "Scene index2: " << mSceneIndex2 << "\n";
    auto numHotSpots = fb.GetUint16LE(); 
    mFlavourText = fb.GetUint32LE(); 
    logger.Spam() << "Hotspots: " << std::dec << numHotSpots << std::endl;
    logger.Spam() << "Flavour Text: " << std::hex << mFlavourText << std::endl;

    const auto moreUnknownData = fb.GetArray<8>();
    logger.Spam() << "More unknown data: " << std::hex 
        << moreUnknownData << std::dec << "\n";

    std::vector<Hotspot> hotspots;

    for (unsigned i = 0; i < numHotSpots; i++)
    {
        auto x = fb.GetUint16LE();
        auto y = fb.GetUint16LE();
        auto w = fb.GetUint16LE();
        auto h = fb.GetUint16LE();
        logger.Spam() << "Hotspot #" << std::dec << i << std::endl;
        const auto unknown = fb.GetUint16LE();
        logger.Spam() << "Unknown: " << std::hex << unknown << std::dec << std::endl;
        const auto keyword = fb.GetUint16LE();
        const auto action = static_cast<HotspotAction>(fb.GetUint16LE());
        logger.Spam() << "Action: " << action << std::endl;
        const auto actionArg1 = fb.GetUint16LE();
        const auto actionArg2 = fb.GetUint16LE();
        const auto actionArg3 = fb.GetUint32LE();
        logger.Spam() << "A1: " << actionArg1 << std::hex << " A2: " << actionArg2 << " A3: " << actionArg3 << std::dec << "\n";
        std::uint32_t tooltip = fb.GetUint32LE(); 
        logger.Spam() << "RightClick: " << std::hex << tooltip << std::endl;
        const auto unknown2 = fb.GetUint32LE();
        logger.Spam() << "Unknown2: " << std::hex << unknown2 << std::dec << std::endl;
        std::uint32_t dialog = fb.GetUint32LE(); 
        logger.Spam() << "LeftClick: " << std::hex << dialog << std::endl;
        const auto unknown3 = fb.GetUint16LE();
        logger.Spam() << "Unknown3: " << std::hex << unknown3 << std::dec << std::endl;

        hotspots.emplace_back(
            i,
            glm::vec<2, int>{x, y},
            glm::vec<2, int>{w, h},
            keyword,
            static_cast<HotspotAction>(action),
            actionArg1,
            actionArg2,
            actionArg3,
            KeyTarget{tooltip},
            KeyTarget{dialog});
    }

    mHotspots = hotspots;

    for (auto& hs : hotspots)
    {
        logger.Spam() << hs << "\n";
    }

    logger.Debug() << "ADS: " << mSceneADS
        << " TTM: " << mSceneTTM
        << " " << mSceneIndex1
        << " " << mSceneIndex2 << "\n";

    auto fb2 = FileBufferFactory::CreateFileBuffer(mSceneADS);
    mAdsIndices = BAK::LoadSceneIndices(fb2);
    auto fb3 = FileBufferFactory::CreateFileBuffer(mSceneTTM);
    mScenes = BAK::LoadScenes(fb3);
}

const Scene& SceneHotspots::GetScene(
    unsigned adsIndex,
    const GameState& gs)
{
    const auto ttmIndex = mAdsIndices[adsIndex];
    return mScenes[ttmIndex.mSceneIndex.GetTTMIndex(gs)];
}

}
