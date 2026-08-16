#include "bak/scene/scene.hpp"

#include "bak/dataTags.hpp"
#include "bak/file/fileBuffer.hpp"
#include "bak/tags.hpp"

#include "com/assert.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"
#include "com/string.hpp"
#include "com/visit.hpp"

#include <sstream>
#include <unordered_map>

namespace BAK {

FileBuffer DecompressSCR(FileBuffer& fb)
{
    auto scrbuf = fb.Find(DataTag::SCR);

    if (scrbuf.GetUint8() != 0x02)
    {
        throw std::runtime_error("Script buffer not compressed");
    }

    auto decompressedSize = scrbuf.GetUint32LE();
    auto decompBuffer = FileBuffer(decompressedSize);
    scrbuf.DecompressLZW(&decompBuffer);

    return decompBuffer;
}

FileBuffer DecompressTT3(FileBuffer& fb)
{
    auto tt3Buffer = fb.Find(DataTag::TT3);

    auto compression = tt3Buffer.GetUint8();
    auto decompressedSize = tt3Buffer.GetUint32LE();
    auto decompBuffer = FileBuffer(decompressedSize);
    if (compression == 1)
    {
        tt3Buffer.DecompressRLE(&decompBuffer);
    }
    else
    {
        tt3Buffer.CopyTo(&decompBuffer, decompressedSize);
    }

    return decompBuffer;
}


std::ostream& operator<<(std::ostream& os, const Script& scene)
{
    os << "Script :: " << scene.mSceneTag << " [\n";
    for (const auto& a : scene.mActions)
    {
        os << '\t' << a << "\n";
    }
    os << " ]\n";
    os << "Script Images: \n";
    for (const auto& [key, imagePal] : scene.mImages)
    {
        const auto& [image, palKey] = imagePal;
        os << "K: " << key << " " << image << " pal: " << palKey << "\n";
    }
    os << "Script Palettes: \n";
    for (const auto& [key, pal] : scene.mPalettes)
    {
        os << "K: " << key << " " << pal << "\n";
    }
    return os;
}

namespace {

std::vector<ScriptAction> DecodeTTM(FileBuffer& fb, Tags& tags)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    auto pageBuffer    = fb.Find(DataTag::PAG);
    auto versionBuffer = fb.Find(DataTag::VER);
    auto tagBuffer     = fb.Find(DataTag::TAG);

    const auto pages = pageBuffer.GetUint16LE();
    logger.Debug() << "Pages:" << pages << " size: " << pageBuffer.GetSize() << "\n";
    logger.Debug() << "Version size: " << versionBuffer.GetSize() << "\n";

    auto decompBuffer = DecompressTT3(fb);
    logger.Debug() << "TT3 size: " << decompBuffer.GetSize() << "\n";

    tags.Load(tagBuffer);
    tags.DumpTags();

    const auto offset = 8 * 3 + pageBuffer.GetSize() + versionBuffer.GetSize() + 5;

    std::vector<ScriptAction> actions{};

    unsigned activeEdgeColor = 0xf;
    unsigned activeFillColor = 0xf;

    while (!decompBuffer.AtEnd())
    {
        unsigned location = decompBuffer.Tell() + offset;
        unsigned int code = decompBuffer.GetUint16LE();
        unsigned int size = code & 0x000f;
        code &= 0xfff0;
        auto action = static_cast<Actions>(code);
        std::stringstream ss{};
        ss << "off: " << std::hex << location << " |Code: " << std::hex << code << " "
            << action << std::dec;

        if ((code == 0x1110) && (size == 1))
        {
            unsigned int id = decompBuffer.GetUint16LE();
            auto name = tags.GetTag(Tag{id});
            if (!name)
            {
                std::stringstream ids{};
                ids << "Unknown[" << id << "]";
                name = ids.str();
            }
            ss << " Name: " << *name << " id [" << id <<"]";

            const auto tag = tags.FindTag(*name);
            actions.emplace_back(
                SetScript{
                    *name,
                    tag
                        ? static_cast<std::uint16_t>(tag->mValue)
                        : static_cast<std::uint16_t>(id)});
            activeEdgeColor = 0xf;
            activeFillColor = 0xf;
        }
        else if (size == 0xf)
        {
            std::string name = ToUpper(decompBuffer.GetString());
            ss << " Name: " << name << " noId";
            if (decompBuffer.GetBytesLeft() & 1) decompBuffer.Skip(1);

            switch (action)
            {
            case Actions::LOAD_PALETTE:
                actions.emplace_back(LoadPalette{name});
                break;
            case Actions::LOAD_IMAGE:
                (*(name.end() - 1)) = 'X';
                actions.emplace_back(LoadImage{name});
                break;
            case Actions::LOAD_SCREEN:
                (*(name.end() - 1)) = 'X';
                actions.emplace_back(LoadScreen{name});
                break;
            default:
                logger.Debug() << "Unhandled action: " << action << "\n";
                break;
            }
        }
        else
        {
            std::vector<std::int16_t> args{};
            for (unsigned int i = 0; i < size; i++)
                args.emplace_back(decompBuffer.GetSint16LE());
            ss << " args [ ";
            auto sep = ' ';
            for (const auto& a : args)
            {
                ss << sep << a;
                sep = ',';
            }
            ss << " ]";

            switch (action)
            {
            case Actions::SLOT_IMAGE:
                actions.emplace_back(SlotImage{static_cast<unsigned>(args[0])});
                break;
            case Actions::SLOT_PALETTE:
                actions.emplace_back(SlotPalette{static_cast<unsigned>(args[0])});
                break;
            case Actions::SET_SCRIPTA: [[fallthrough]];
            case Actions::SET_SCRIPT:
            {
                ASSERT(!args.empty());
                const auto tag = tags.GetTag(Tag{static_cast<unsigned>(args[0])});
                const auto sceneTag = tag
                    ? *tag
                    : std::to_string(static_cast<int>(args[0]));
                actions.emplace_back(
                    SetScript{
                        sceneTag,
                        static_cast<std::uint16_t>(args[0])});
            } break;
            case Actions::SET_CLIP_REGION:
                actions.emplace_back(
                    ClipRegion{
                        glm::vec2{args[0], args[1]},
                        glm::vec2{args[2], args[3]}});
                break;
            case Actions::SAVE_IMAGE0:
                actions.emplace_back(
                    SaveImage{
                        glm::vec2{args[0], args[1]},
                        glm::vec2{args[2], args[3]}});
                break;
            case Actions::SAVE_REGION_TO_LAYER:
                actions.emplace_back(
                    SaveRegionToLayer{
                        glm::vec2{args[0], args[1]},
                        glm::vec2{args[2], args[3]}});
                break;
            case Actions::DRAW_SCREEN:
                actions.emplace_back(
                    DrawScreen{
                        glm::vec2{args[0], args[1]},
                        glm::vec2{args[2], args[3]},
                        static_cast<unsigned>(args[4]),
                        static_cast<unsigned>(args[5])});
                break;
            case Actions::DRAW_RECT: [[fallthrough]];
            case Actions::DRAW_FRAME:
                actions.emplace_back(
                    DrawRect{
                        activeEdgeColor,
                        activeFillColor,
                        glm::vec2{args[0], args[1]},
                        glm::vec2{args[2], args[3]},
                        action == Actions::DRAW_RECT});
                break;
            // FIXME: Implement the rotation
            case Actions::DRAW_SPRITE_ROTATE: [[fallthrough]];
            case Actions::DRAW_SPRITE_FLIP_XY: [[fallthrough]];
            case Actions::DRAW_SPRITE_FLIP_X: [[fallthrough]];
            case Actions::DRAW_SPRITE_FLIP_Y: [[fallthrough]];
            case Actions::DRAW_SPRITE:
            {
                const auto flipXY = action == Actions::DRAW_SPRITE_ROTATE
                    ? 0u
                    : (code & 0x00f0) >> 4;
                const auto scaled = args.size() >= 6;
                static constexpr auto FLIP_X = 2;
                static constexpr auto FLIP_Y = 1;
                actions.emplace_back(
                    DrawSprite{
                        (flipXY & FLIP_X) != 0,
                        (flipXY & FLIP_Y) != 0,
                        args[0],
                        args[1],
                        args[2],
                        args[3],
                        static_cast<std::int16_t>(scaled ? args[4] : 0),
                        static_cast<std::int16_t>(scaled ? args[5] : 0)});
            } break;
            case Actions::PLAY_SOUND:
                actions.emplace_back(PlaySoundS{static_cast<unsigned>(args[0])});
                break;
            case Actions::SAVE_BACKGROUND:
                actions.emplace_back(SaveBackground{});
                break;
            case Actions::SET_COLOR:
                activeEdgeColor = static_cast<unsigned>(args[0]);
                activeFillColor = static_cast<unsigned>(args[1]);
                actions.emplace_back(
                    SetColors{activeEdgeColor, activeFillColor});
                break;
            case Actions::FADE_IN:
                actions.emplace_back(FadeIn{});
                break;
            case Actions::FADE_OUT:
                actions.emplace_back(FadeOut{});
                break;
            case Actions::GOTO_TAG:
                actions.emplace_back(GotoTag{static_cast<unsigned>(args[0])});
                break;
            case Actions::SHOW_DIALOG:
                actions.emplace_back(
                    ShowDialog{
                        args[0] == -1
                            ? std::nullopt
                            : std::optional<unsigned>{static_cast<unsigned>(args[0])},
                        static_cast<unsigned>(args[1])});
                break;
            case Actions::SET_SAVE_LAYER:
                actions.emplace_back(SetSaveLayer{static_cast<unsigned>(args[0])});
                break;
            case Actions::DRAW_SAVED_REGION:
                actions.emplace_back(DrawSavedRegion{static_cast<unsigned>(args[0])});
                break;
            case Actions::UPDATE:
                actions.emplace_back(Update{});
                break;
            case Actions::PURGE:
                actions.emplace_back(Purge{});
                break;
            case Actions::DELAY:
                actions.emplace_back(Delay{static_cast<unsigned>(args[0])});
                break;
            default:
                logger.Debug() << "Unhandled action: " << action << "\n";
                break;
            }
        }

        logger.Debug() << ss.str() << "\n";
    }

    return actions;
}

}

std::unordered_map<unsigned, Script> LoadScripts(FileBuffer& fb)
{
    Tags tags{};
    const auto actions = DecodeTTM(fb, tags);

    std::unordered_map<unsigned, Script> scripts{};

    Script currentScript;
    bool loadingScene = false;
    std::optional<unsigned> imageSlot = 0;
    std::optional<PaletteSlot> paletteSlot{};
    std::unordered_map<unsigned, std::string> palettes{};
    std::unordered_map<
        unsigned,
        std::pair<std::string, unsigned>> images{};
    // FIXME: Probably a nicer way of doing image slots.
    std::unordered_map<
        unsigned,
        ImageSlot> imageSlots;

    std::unordered_map<
        unsigned,
        std::pair<std::string, unsigned>> screens{};

    const auto PushScript = [&]{
        currentScript.mPalettes = palettes;
        currentScript.mImages.clear();
        for (auto [key, val] : images)
        {
            if (!currentScript.mPalettes.contains(val.second))
            {
                val = std::make_pair(val.first, 0);
            }
            currentScript.mImages[key] = val;
        }
        currentScript.mScreens = screens;
        currentScript.mActions.emplace_back(DisableClipRegion{});

        const auto tag = tags.FindTag(currentScript.mSceneTag);
        if (tag)
        {
            scripts[tag->mValue] = currentScript;
        }
        else
        {
            throw std::runtime_error("Tag not found");
        }
    };

    for (const auto& action : actions)
    {
        std::visit(
            overloaded{
                [&](const SetScript& setScript)
                {
                    if (loadingScene)
                        PushScript();

                    currentScript.mSceneTag = setScript.mName;
                    currentScript.mActions.clear();
                    currentScript.mImages.clear();
                    currentScript.mScreens.clear();
                    currentScript.mPalettes.clear();
                    images.clear();
                    imageSlots.clear();
                    palettes.clear();
                    imageSlot.reset();
                    screens.clear();
                    loadingScene = true;
                },
                [&](const SlotImage& slotImage)
                {
                    imageSlot = slotImage.mSlot;
                },
                [&](const SlotPalette& slotPalette)
                {
                    paletteSlot = slotPalette.mSlot;
                },
                [&](const LoadPalette& loadPalette)
                {
                    //ASSERT(loadingScene);
                    ASSERT(paletteSlot);
                    palettes[*paletteSlot] = loadPalette.mPalette;
                    if (!imageSlots.contains(*paletteSlot))
                    {
                        imageSlots[*paletteSlot] = ImageSlot{};
                    }
                    imageSlots[*paletteSlot].mPalette = *paletteSlot;
                },
                [&](const LoadImage& loadImage)
                {
                    //ASSERT(loadingScene);
                    ASSERT(imageSlot);
                    images[*imageSlot] = std::make_pair(
                        loadImage.mImage,
                        paletteSlot ? *paletteSlot : *imageSlot);
                    if (!imageSlots.contains(*imageSlot))
                    {
                        imageSlots[*imageSlot] = ImageSlot{};
                    }
                },
                [&](const LoadScreen& loadScreen)
                {
                    if (!paletteSlot) return;
                    screens[*paletteSlot] = std::make_pair(
                        loadScreen.mScreenName,
                        *paletteSlot);
                },
                [&](const ClipRegion& a)
                {
                    currentScript.mActions.emplace_back(a);
                },
                [&](const DrawScreen& a)
                {
                    currentScript.mActions.emplace_back(a);
                },
                [&](const DrawRect& a)
                {
                    currentScript.mActions.emplace_back(a);
                },
                [&](const DrawSprite& a)
                {
                    currentScript.mActions.emplace_back(a);
                },
                [&](const ShowDialog& a)
                {
                    currentScript.mActions.emplace_back(a);
                },
                [&](const Update& a)
                {
                    currentScript.mActions.emplace_back(a);
                },
                [&](const Purge& a)
                {
                    currentScript.mActions.emplace_back(a);
                },
                [&](const Delay& a)
                {
                    currentScript.mActions.emplace_back(a);
                },
                [](const auto&){}},
            action);
    }

    // Push final scene
    PushScript();

    return scripts;
}

std::vector<ScriptAction> LoadDynamicScripts(FileBuffer& fb)
{
    Tags tags{};
    return DecodeTTM(fb, tags);
}

FileBuffer DecompressTTM(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);


    auto pageBuffer    = fb.Find(DataTag::PAG);
    auto versionBuffer = fb.Find(DataTag::VER);
    auto tt3Buffer     = fb.Find(DataTag::TT3);
    auto tagBuffer     = fb.Find(DataTag::TAG);
    
    const auto pages = pageBuffer.GetUint16LE();
    logger.Debug() << "Pages:" << pages << " size: " << pageBuffer.GetSize() << "\n";

    logger.Debug() << "Version: " << versionBuffer.GetString() << "\n";

    auto compression = tt3Buffer.GetUint8();
    logger.Debug() << "Compression: " << +compression << "\n";
    auto size = tt3Buffer.GetUint32LE();
    logger.Debug() << "Decompressed size: " << size << "\n";
    FileBuffer decompBuffer = FileBuffer(size);
    auto decomped = tt3Buffer.DecompressRLE(&decompBuffer);
    logger.Debug() << "Decompressed bytes: " << decomped << "\n";


    Tags tags{};
    tags.Load(tagBuffer);
    tags.DumpTags();

    auto decompressedTTM = FileBuffer(
        pageBuffer.GetSize() + 8
        + versionBuffer.GetSize() + 8
        + tagBuffer.GetSize() + 8 
        + 1 + 4 + decomped + 8);


    decompressedTTM.PutUint32LE(static_cast<std::uint32_t>(DataTag::VER));
    decompressedTTM.PutUint32LE(versionBuffer.GetSize());
    versionBuffer.Rewind();
    versionBuffer.CopyTo(&decompressedTTM, versionBuffer.GetSize());

    decompressedTTM.PutUint32LE(static_cast<std::uint32_t>(DataTag::PAG));
    decompressedTTM.PutUint32LE(pageBuffer.GetSize());
    pageBuffer.Rewind();
    pageBuffer.CopyTo(&decompressedTTM, pageBuffer.GetSize());

    decompressedTTM.PutUint32LE(static_cast<std::uint32_t>(DataTag::TT3));
    decompressedTTM.PutUint32LE(1 + 4 + decomped);
    decompressedTTM.PutUint8(0);
    decompressedTTM.PutUint32LE(decomped);
    decompBuffer.Rewind();
    decompBuffer.CopyTo(&decompressedTTM, decompBuffer.GetSize());

    decompressedTTM.PutUint32LE(static_cast<std::uint32_t>(DataTag::TAG));
    decompressedTTM.PutUint32LE(tagBuffer.GetSize());
    tagBuffer.Rewind();
    tagBuffer.CopyTo(&decompressedTTM, tagBuffer.GetSize());

    return decompressedTTM;
}

namespace {

template <typename T>
std::string ActionName(unsigned code)
{
    try
    {
        return std::string{ToString(static_cast<T>(code))};
    }
    catch (const std::runtime_error&)
    {
        std::stringstream ss{};
        ss << "Unknown(0x" << std::hex << code << std::dec << ")";
        return ss.str();
    }
}

}

void DumpADS(FileBuffer& fb, std::ostream& os)
{
    auto decompBuffer = DecompressSCR(fb);

    std::optional<unsigned> currentIndex{};
    while (!decompBuffer.AtEnd())
    {
        if (!currentIndex)
        {
            currentIndex = decompBuffer.GetUint16LE();
            os << "Index: " << *currentIndex << "\n";
        }

        auto code = decompBuffer.GetUint16LE();
        auto action = static_cast<AdsActions>(code);
        os << "\t" << ActionName<AdsActions>(code);

        switch (action)
        {
            case AdsActions::IF_NOT_PLAYED: [[fallthrough]];
            case AdsActions::IF_NOT_PLAYED_ELSE: [[fallthrough]];
            case AdsActions::IF_PLAYED_ELSE:
            {
                os << " " << decompBuffer.GetUint16LE()
                    << " " << decompBuffer.GetUint16LE();
            } break;
            case AdsActions::RESTART_SCRIPT: [[fallthrough]];
            case AdsActions::START_SCRIPT:
            {
                os << " " << decompBuffer.GetUint16LE()
                    << " " << decompBuffer.GetUint16LE()
                    << " " << decompBuffer.GetUint16LE()
                    << " " << decompBuffer.GetUint16LE();
            } break;
            case AdsActions::STOP_SCRIPT:
            {
                os << " " << decompBuffer.GetUint16LE()
                    << " " << decompBuffer.GetUint16LE()
                    << " " << decompBuffer.GetUint16LE();
            } break;
            case AdsActions::IF_CHAP_GTE: [[fallthrough]];
            case AdsActions::IF_CHAP_LTE: [[fallthrough]];
            case AdsActions::STOP_SCENE:
            {
                os << " " << decompBuffer.GetUint16LE();
            } break;
            case AdsActions::AND: [[fallthrough]];
            case AdsActions::OR: [[fallthrough]];
            case AdsActions::ELSE: [[fallthrough]];
            case AdsActions::END_IF_ELSE: [[fallthrough]];
            case AdsActions::END_IF:
                break;
            case AdsActions::END:
                currentIndex.reset();
                break;
        }

        os << "\n";
    }
}

void DumpTTM(FileBuffer& fb, std::ostream& os)
{
    auto tagBuffer = fb.Find(DataTag::TAG);
    Tags tags{};
    tags.Load(tagBuffer);

    auto decompBuffer = DecompressTT3(fb);

    while (!decompBuffer.AtEnd())
    {
        auto code = decompBuffer.GetUint16LE();
        auto size = static_cast<unsigned>(code & 0x000f);
        code &= 0xfff0;
        auto action = static_cast<Actions>(code);

        os << ActionName<Actions>(code);

        if ((code == 0x1110) && (size == 1))
        {
            auto id = decompBuffer.GetUint16LE();
            const auto name = tags.GetTag(Tag{id});
            os << " Id: " << id;
            if (name)
                os << " Name: " << *name;
        }
        else if (size == 0xf)
        {
            std::string name = ToUpper(decompBuffer.GetString());
            os << " Name: " << name;
            if (decompBuffer.GetBytesLeft() & 1)
                decompBuffer.Skip(1);
        }
        else
        {
            os << " args [";
            for (unsigned i = 0; i < size; i++)
                os << " " << decompBuffer.GetSint16LE();
            os << " ]";
        }

        os << "\n";
    }
}

}
