#include "bak/dataTags.hpp"
#include "bak/scene.hpp"
#include "bak/tags.hpp"

#include "bak/dialogSources.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"
#include "com/string.hpp"

#include <unordered_map>

namespace BAK {


std::ostream& operator<<(std::ostream& os, const ADSIndex& ai)
{
    os << "ADSIndex{ if (" << ai.mLessThan << " <= [chapter] <= "
        << ai.mGreaterThan << ") then " << ai.mIf << " else " << ai.mElse;
    return os;
}

std::ostream& operator<<(std::ostream& os, const SceneIndex& si)
{
    os << "SceneIndex{ tag: " << si.mSceneTag << " index: " << si.mSceneIndex << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Scene& scene)
{
    os << "Scene :: " << scene.mSceneTag << " [\n";
    for (const auto& a : scene.mActions)
    {
        os << '\t' << a << "\n";
    }
    os << " ]\n";
    os << "Scene Images: \n";
    for (const auto& [key, imagePal] : scene.mImages)
    {
        const auto& [image, palKey] = imagePal;
        os << "K: " << key << " " << image << " pal: " << palKey << "\n";
    }
    os << "Scene Palettes: \n";
    for (const auto& [key, pal] : scene.mPalettes)
    {
        os << "K: " << key << " " << pal << "\n";
    }
    return os;
}

std::unordered_map<unsigned, SceneIndex> LoadSceneIndices(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    auto resbuf = fb.Find(DataTag::RES);
    auto scrbuf = fb.Find(DataTag::SCR);
    auto tagbuf = fb.Find(DataTag::TAG);

    if (scrbuf.GetUint8() != 0x02)
    {
        throw std::runtime_error("Script buffer not compressed");
    }

    auto decompressedSize = scrbuf.GetUint32LE();
    auto script = FileBuffer(decompressedSize);
    scrbuf.DecompressLZW(&script);
    
    Tags tags{};
    tags.Load(tagbuf);

    std::optional<unsigned> currentIndex{};
    std::optional<unsigned> ttmIndex{};
    auto adsIndex = ADSIndex{};
    bool inElseClause = false;

    std::unordered_map<unsigned, SceneIndex> sceneIndices;

    while (!script.AtEnd())
    {
        if (!currentIndex)
        {
            currentIndex = script.GetUint16LE();
            logger.Debug() << "Index: " << currentIndex << "\n";
        }

        auto code = script.GetUint16LE();
        auto action = static_cast<AdsActions>(code);
        std::stringstream ss{};
        ss << std::hex << code << std::dec << " adsAct: " << action << " ";
        
        switch (action)
        {
            case AdsActions::INDEX:
            {
                const auto a = script.GetUint16LE();
                const auto b = script.GetUint16LE();
                ss << a << " " << b;
            }
                break;
            case AdsActions::IF_NOT_PLAYED: [[fallthrough]];
            case AdsActions::IF_PLAYED: 
            {
                const auto a = script.GetUint16LE();
                const auto b = script.GetUint16LE();
                ss << a << " " << b;
            }
                break;
            case AdsActions::ADD_SCENE2: [[fallthrough]];
            case AdsActions::ADD_SCENE:
            {
                const auto a = script.GetUint16LE();
                const auto b = script.GetUint16LE();
                const auto c = script.GetUint16LE();
                const auto d = script.GetUint16LE();
                ss << a << " " << b << " " << c << " " << d;
                ttmIndex = std::max(
                    std::max(a, b),
                    std::max(c, d));

                if (!inElseClause)
                    adsIndex.mIf = *ttmIndex;
                else
                    adsIndex.mElse = *ttmIndex;
            }
                break;
            case AdsActions::END:
            {
                ASSERT(currentIndex);
                ASSERT(ttmIndex);

                const auto sceneName = tags.GetTag(Tag{*currentIndex});
                if (!sceneName)
                    throw std::runtime_error("Tag not found");
                sceneIndices[*currentIndex] = SceneIndex{
                    *sceneName,
                    adsIndex};

                // Reset the state for next scene index
                currentIndex.reset();
                ttmIndex.reset();
                adsIndex = ADSIndex{};
                inElseClause = false;
            }
                break;
            case AdsActions::STOP_SCENE:
            {
                const auto a = script.GetUint16LE();
                const auto b = script.GetUint16LE();
                const auto c = script.GetUint16LE();
                ss << a << " " << b << " " << c;
            }
                break;
            case AdsActions::IF_CHAP_GTE:
            {
                const auto a = script.GetUint16LE();
                ss << a;
                adsIndex.mGreaterThan = a;
            }
                break;
            case AdsActions::IF_CHAP_LTE:
            {
                const auto a = script.GetUint16LE();
                ss << a;
                adsIndex.mLessThan = a;
            }
                break;
            case AdsActions::ELSE:
                inElseClause = true;
                break;
            // Currently only chapter conditions are implemented
            // so and and or are not implemented...
            case AdsActions::AND: [[fallthrough]];
            case AdsActions::OR: [[fallthrough]];
            case AdsActions::FADE_OUT: [[fallthrough]];
            case AdsActions::END_IF: [[fallthrough]];
            case AdsActions::PLAY_SCENE: [[fallthrough]];
            case AdsActions::PLAY_ALL_SCENES:
                break;
        }

        logger.Debug() << ss.str() << "\n";
    }

    return sceneIndices;
}

std::unordered_map<unsigned, std::vector<SceneSequence>> LoadSceneSequences(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    auto resbuf = fb.Find(DataTag::RES);
    auto scrbuf = fb.Find(DataTag::SCR);
    auto tagbuf = fb.Find(DataTag::TAG);

    if (scrbuf.GetUint8() != 0x02)
    {
        throw std::runtime_error("Script buffer not compressed");
    }

    auto decompressedSize = scrbuf.GetUint32LE();
    auto script = FileBuffer(decompressedSize);
    scrbuf.DecompressLZW(&script);
    
    Tags tags{};
    tags.Load(tagbuf);

    std::optional<unsigned> currentIndex{};

    std::unordered_map<unsigned, std::vector<SceneSequence>> sceneIndices;
    SceneSequence currentSequence{};

    while (!script.AtEnd())
    {
        if (!currentIndex)
        {
            currentIndex = script.GetUint16LE();
            sceneIndices.emplace(*currentIndex, std::vector<SceneSequence>({}));
            logger.Debug() << "Index: " << currentIndex << "\n";
        }

        auto code = script.GetUint16LE();
        auto action = static_cast<AdsActions>(code);
        std::stringstream ss{};
        ss << std::hex << code << std::dec << " adsAct: " << action << " ";
        
        switch (action)
        {
            case AdsActions::INDEX:
            {
                const auto a = script.GetUint16LE();
                const auto b = script.GetUint16LE();
                ss << a << " " << b;
            }
                break;
            case AdsActions::IF_NOT_PLAYED: [[fallthrough]];
            case AdsActions::IF_PLAYED: 
            {
                const auto a = script.GetUint16LE();
                const auto b = script.GetUint16LE();
                ss << a << " " << b;
            }
                break;
            case AdsActions::ADD_SCENE2: [[fallthrough]];
            case AdsActions::ADD_SCENE:
            {
                const auto a = script.GetUint16LE();
                const auto b = script.GetUint16LE();
                const auto c = script.GetUint16LE();
                const auto d = script.GetUint16LE();
                ss << a << " " << b << " " << c << " " << d;
                ASSERT(currentIndex);
                currentSequence.mScenes.emplace_back(SceneADS(a, b, false));
            }
                break;
            case AdsActions::PLAY_SCENE:
            {
                ASSERT(currentIndex);
                const auto sceneName = tags.GetTag(Tag{*currentIndex});
                if (!sceneName)
                    throw std::runtime_error("Tag not found");

                // Reset the state for next scene index
                sceneIndices.at(*currentIndex).emplace_back(currentSequence);
                currentSequence = {};
            } break;
            case AdsActions::PLAY_ALL_SCENES:
            {
                ASSERT(currentIndex);
                const auto sceneName = tags.GetTag(Tag{*currentIndex});
                if (!sceneName)
                    throw std::runtime_error("Tag not found");
                currentSequence = {};
                currentSequence.mScenes.emplace_back(SceneADS(0, 0, true));
                // Reset the state for next scene index
                sceneIndices.at(*currentIndex).emplace_back(currentSequence);
            } break;
            case AdsActions::END:
            {
                currentIndex.reset();
            } break;
            case AdsActions::STOP_SCENE:
            {
                const auto a = script.GetUint16LE();
                const auto b = script.GetUint16LE();
                const auto c = script.GetUint16LE();
                ss << a << " " << b << " " << c;
            } break;
            case AdsActions::IF_CHAP_GTE:
            {
                assert(false);
            } break;
            case AdsActions::IF_CHAP_LTE:
            {
                assert(false);
            }
                break;
            case AdsActions::ELSE:
                assert(false);
                break;
            // Currently only chapter conditions are implemented
            // so and and or are not implemented...
            case AdsActions::AND: [[fallthrough]];
            case AdsActions::OR: [[fallthrough]];
            case AdsActions::FADE_OUT: [[fallthrough]];
            case AdsActions::END_IF:
                break;
        }

        logger.Debug() << ss.str() << "\n";
    }

    return sceneIndices;
}



std::unordered_map<unsigned, Scene> LoadScenes(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    std::vector<SceneChunk> chunks;

    auto pageBuffer    = fb.Find(DataTag::PAG);
    auto versionBuffer = fb.Find(DataTag::VER);
    auto tt3Buffer     = fb.Find(DataTag::TT3);
    auto tagBuffer     = fb.Find(DataTag::TAG);
    
    const auto pages = pageBuffer.GetUint16LE();
    logger.Debug() << "Pages:" << pages << " size: " << pageBuffer.GetSize() << "\n";

    logger.Debug() << "Version size: " << versionBuffer.GetSize() << "\n";

    tt3Buffer.Skip(1);
    FileBuffer decompBuffer = FileBuffer(tt3Buffer.GetUint32LE());
    auto decomped = tt3Buffer.DecompressRLE(&decompBuffer);

    Tags tags{};
    tags.Load(tagBuffer);

    while (!decompBuffer.AtEnd())
    {
        unsigned int code = decompBuffer.GetUint16LE();
        unsigned int size = code & 0x000f;
        code &= 0xfff0;
        auto action = static_cast<Actions>(code);
        std::stringstream ss{};
        ss << "Code: " << std::hex << code << " " 
            << action << std::dec;
        
        if ((code == 0x1110) && (size == 1))
        {
            unsigned int id = decompBuffer.GetUint16LE();
            if (const auto name = tags.GetTag(Tag{id}))
            {
                ss << " Name: " << *name;
                chunks.emplace_back(action, name, std::vector<std::int16_t>{});
            }
        }
        else if (size == 0xf)
        {
            std::string name = ToUpper(decompBuffer.GetString());
            ss << " Name: " << name;
            if (decompBuffer.GetBytesLeft() & 1) decompBuffer.Skip(1);
            chunks.emplace_back(action, name, std::vector<std::int16_t>{});
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
            chunks.emplace_back(
                action,
                std::optional<std::string>{},
                args);
        }

        logger.Debug() << ss.str() << "\n";
    }

    std::unordered_map<unsigned, Scene> scenes{};

    Scene currentScene;
    bool loadingScene = false;
    bool flipped = false;
    std::optional<unsigned> imageSlot = 0;
    std::optional<PaletteSlot> paletteSlot{};
    std::optional<std::pair<unsigned, unsigned>> activeColor{};
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

    const auto PushScene = [&]{
        currentScene.mPalettes = palettes;
        currentScene.mImages.clear();
        for (auto [key, val] : images)
        {
            if (!currentScene.mPalettes.contains(val.second))
            {
                val = std::make_pair(val.first, 0);
            }
            currentScene.mImages[key] = val;
        }
        currentScene.mScreens = screens;
        currentScene.mActions.emplace_back(DisableClipRegion{});

        const auto tag = tags.FindTag(currentScene.mSceneTag);
        if (tag)
        {
            scenes[tag->mValue] = currentScene;
        }
        else
        {
            throw std::runtime_error("Tag not found");
        }
    };

    for (const auto& chunk : chunks)
    {
        switch (chunk.mAction)
        {
        case Actions::SLOT_IMAGE:
            imageSlot = chunk.mArguments[0];
            break;
        case Actions::SLOT_PALETTE:
            paletteSlot = chunk.mArguments[0];
            break;
        case Actions::SET_SCENE:
            if (loadingScene)
                PushScene();

            ASSERT(chunk.mResourceName);
            currentScene.mSceneTag = *chunk.mResourceName;
            currentScene.mActions.clear();
            currentScene.mImages.clear();
            currentScene.mScreens.clear();
            currentScene.mPalettes.clear();
            images.clear();
            imageSlots.clear();
            palettes.clear();
            imageSlot.reset();
            activeColor.reset();
            screens.clear();
            loadingScene = true;
            break;

        case Actions::SET_CLIP_REGION:
            // Transform this to opengl coords...
            currentScene.mActions.emplace_back(
                ClipRegion{
                    glm::vec2{chunk.mArguments[0], chunk.mArguments[1]},
                    glm::vec2{chunk.mArguments[2], chunk.mArguments[3]}});
            break;

        case Actions::LOAD_PALETTE:
        {
            //ASSERT(loadingScene);
            ASSERT(chunk.mResourceName);
            ASSERT(paletteSlot);
            const auto paletteName = *chunk.mResourceName;
            palettes[*paletteSlot] = paletteName;
            if (!imageSlots.contains(*paletteSlot))
            {
                imageSlots[*paletteSlot] = ImageSlot{};
            }
            imageSlots[*paletteSlot].mPalette = *paletteSlot;
        } break;

        case Actions::SET_COLOR:
            ASSERT(paletteSlot);
            activeColor = std::make_pair(
                *paletteSlot,
                chunk.mArguments[0]);
            break;

        case Actions::LOAD_IMAGE:
        {
            //ASSERT(loadingScene);
            ASSERT(chunk.mResourceName);
            ASSERT(imageSlot);
            auto name = *chunk.mResourceName;
            (*(name.end() - 1)) = 'X';
            images[*imageSlot] = std::make_pair(name, paletteSlot ? *paletteSlot: *imageSlot);
            if (!imageSlots.contains(*imageSlot))
            {
                imageSlots[*imageSlot] = ImageSlot{};
            }
            //imageSlots[*imageSlot].mImage = name;
        } break;
        case Actions::LOAD_SCREEN:
        {
            ASSERT(chunk.mResourceName);
            ASSERT(paletteSlot);
            auto name = *chunk.mResourceName;
            (*(name.end() - 1)) = 'X';
            screens[*paletteSlot] = std::make_pair(name, *paletteSlot);
        } break;
        case Actions::DRAW_SCREEN:
        {
            currentScene.mActions.emplace_back(
                DrawScreen{
                    glm::vec2{chunk.mArguments[0], chunk.mArguments[1]},
                    glm::vec2{chunk.mArguments[2], chunk.mArguments[3]},
                });
        } break;
        case Actions::DRAW_RECT:
        case Actions::DRAW_FRAME:
            // There must be a default colour?
            //ASSERT(activeColor);
            currentScene.mActions.emplace_back(
                DrawRect{
                    *activeColor,
                    glm::vec2{chunk.mArguments[0], chunk.mArguments[1]},
                    glm::vec2{chunk.mArguments[2], chunk.mArguments[3]}});
            break;
        // FIXME: Implement the rotation
        case Actions::DRAW_SPRITE_ROTATE: [[fallthrough]];
        // FIXME: Implement flip in x and y
        case Actions::DRAW_SPRITE_FLIP_XY: [[fallthrough]];
        case Actions::DRAW_SPRITE1: [[fallthrough]];
        case Actions::DRAW_SPRITE_FLIP_Y:
            flipped = true;
            [[fallthrough]];
        case Actions::DRAW_SPRITE0:
        {
            const auto scaled = chunk.mArguments.size() >= 5;
            currentScene.mActions.emplace_back(
                DrawSprite{
                    flipped,
                    chunk.mArguments[0],
                    chunk.mArguments[1],
                    chunk.mArguments[2],
                    chunk.mArguments[3],
                    static_cast<std::int16_t>(scaled ? chunk.mArguments[4] : 0),
                    static_cast<std::int16_t>(scaled ? chunk.mArguments[5] : 0)});
            flipped = false;
        } break;
        case Actions::UPDATE:
        {
            currentScene.mActions.emplace_back(Update{});
        } break;
        case Actions::PURGE:
        {
            currentScene.mActions.emplace_back(Purge{});
        } break;
        case Actions::DELAY:
        {
            currentScene.mActions.emplace_back(
                Delay{static_cast<unsigned>(chunk.mArguments[0])});
        } break;
        default:
            logger.Debug() << "Unhandled action: " << chunk.mAction << "\n";
            break;
        }
    }

    // Push final scene
    PushScene();

    return scenes;
}

std::map<unsigned, DynamicScene> LoadDynamicScenes(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    std::vector<SceneChunk> chunks;

    auto pageBuffer    = fb.Find(DataTag::PAG);
    auto versionBuffer = fb.Find(DataTag::VER);
    auto tt3Buffer     = fb.Find(DataTag::TT3);
    auto tagBuffer     = fb.Find(DataTag::TAG);
    
    const auto pages = pageBuffer.GetUint16LE();
    logger.Debug() << "Pages:" << pages << " size: " << pageBuffer.GetSize() << "\n";

    logger.Debug() << "Version size: " << versionBuffer.GetSize() << "\n";

    tt3Buffer.Skip(1);
    FileBuffer decompBuffer = FileBuffer(tt3Buffer.GetUint32LE());
    auto decomped = tt3Buffer.DecompressRLE(&decompBuffer);

    Tags tags{};
    tags.Load(tagBuffer);

    while (!decompBuffer.AtEnd())
    {
        unsigned int code = decompBuffer.GetUint16LE();
        unsigned int size = code & 0x000f;
        code &= 0xfff0;
        auto action = static_cast<Actions>(code);
        std::stringstream ss{};
        ss << "Code: " << std::hex << code << " " 
            << action << std::dec;
        
        if ((code == 0x1110) && (size == 1))
        {
            unsigned int id = decompBuffer.GetUint16LE();
            if (const auto name = tags.GetTag(Tag{id}))
            {
                ss << " Name: " << *name;
                chunks.emplace_back(action, name, std::vector<std::int16_t>{});
            }
        }
        else if (size == 0xf)
        {
            std::string name = ToUpper(decompBuffer.GetString());
            ss << " Name: " << name;
            if (decompBuffer.GetBytesLeft() & 1) decompBuffer.Skip(1);
            chunks.emplace_back(action, name, std::vector<std::int16_t>{});
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
            chunks.emplace_back(
                action,
                std::optional<std::string>{},
                args);
        }

        logger.Debug() << ss.str() << "\n";
    }

    std::map<unsigned, DynamicScene> scenes{};

    DynamicScene currentScene;
    bool loadingScene = false;
    bool flipped = false;

    const auto PushScene = [&]{
        currentScene.mActions.emplace_back(DisableClipRegion{});

        const auto tag = tags.FindTag(currentScene.mSceneTag);
        if (tag)
        {
            scenes[tag->mValue] = currentScene;
        }
        else
        {
            throw std::runtime_error("Tag not found");
        }
    };

    for (const auto& chunk : chunks)
    {
        switch (chunk.mAction)
        {
        case Actions::SLOT_IMAGE:
            currentScene.mActions.emplace_back(SlotImage{static_cast<unsigned>(chunk.mArguments[0])});
            break;
        case Actions::SLOT_PALETTE:
            currentScene.mActions.emplace_back(SlotPalette{static_cast<unsigned>(chunk.mArguments[0])});
            break;
        case Actions::SET_SCENE:
            if (loadingScene)
                PushScene();

            ASSERT(chunk.mResourceName);
            currentScene.mSceneTag = *chunk.mResourceName;
            currentScene.mActions.clear();
            loadingScene = true;
            break;

        case Actions::SET_CLIP_REGION:
            // Transform this to opengl coords...
            currentScene.mActions.emplace_back(
                ClipRegion{
                    glm::vec2{chunk.mArguments[0], chunk.mArguments[1]},
                    glm::vec2{chunk.mArguments[2], chunk.mArguments[3]}});
            break;

        case Actions::LOAD_PALETTE:
        {
            const auto paletteName = *chunk.mResourceName;
            currentScene.mActions.emplace_back(LoadPalette{paletteName});
        } break;

        case Actions::SAVE_IMAGE0: [[fallthrough]];
        case Actions::SAVE_IMAGE1:
        {
            currentScene.mActions.emplace_back(
                SaveImage{
                    glm::vec2{chunk.mArguments[0], chunk.mArguments[1]},
                    glm::vec2{chunk.mArguments[2], chunk.mArguments[3]}});
        } break;

        case Actions::SAVE_BACKGROUND:
        {
            currentScene.mActions.emplace_back(
                SaveBackground{});
        } break;

        case Actions::SET_COLOR:
            //ASSERT(paletteSlot);
            //activeColor = std::make_pair(
            //    *paletteSlot,
            //    chunk.mArguments[0]);
            break;
        case Actions::LOAD_IMAGE:
        {
            auto imageName = *chunk.mResourceName;
            (*(imageName.end() - 1)) = 'X';
            currentScene.mActions.emplace_back(LoadImage{imageName});
        } break;
        case Actions::LOAD_SCREEN:
        {
            ASSERT(chunk.mResourceName);
            auto name = *chunk.mResourceName;
            (*(name.end() - 1)) = 'X';
            currentScene.mActions.emplace_back(LoadScreen{name});
        } break;
        case Actions::DRAW_SCREEN:
        {
            currentScene.mActions.emplace_back(
                DrawScreen{
                    glm::vec2{chunk.mArguments[0], chunk.mArguments[1]},
                    glm::vec2{chunk.mArguments[2], chunk.mArguments[3]},
                });
        } break;
        case Actions::FADE_IN:
        {
            currentScene.mActions.emplace_back(FadeIn{});
        } break;
        case Actions::FADE_OUT:
        {
            currentScene.mActions.emplace_back(FadeOut{});
        } break;
        case Actions::SHOW_DIALOG:
        {
            currentScene.mActions.emplace_back(
                ShowDialog{DialogSources::GetTTMDialogKey(chunk.mArguments[0])});
        } break;
        case Actions::DRAW_RECT:
        case Actions::DRAW_FRAME:
            currentScene.mActions.emplace_back(
                DrawRect{
                    std::make_pair(0,0), // active color
                    glm::vec2{chunk.mArguments[0], chunk.mArguments[1]},
                    glm::vec2{chunk.mArguments[2], chunk.mArguments[3]}});
            break;
        // FIXME: Implement the rotation
        case Actions::DRAW_SPRITE_ROTATE: [[fallthrough]];
        // FIXME: Implement flip in x and y
        case Actions::DRAW_SPRITE_FLIP_XY: [[fallthrough]];
        case Actions::DRAW_SPRITE1: [[fallthrough]];
        case Actions::DRAW_SPRITE_FLIP_Y:
            flipped = true;
            [[fallthrough]];
        case Actions::DRAW_SPRITE0:
        {
            const auto scaled = chunk.mArguments.size() >= 5;
            currentScene.mActions.emplace_back(
                DrawSprite{
                    flipped,
                    chunk.mArguments[0],
                    chunk.mArguments[1],
                    chunk.mArguments[2],
                    chunk.mArguments[3],
                    static_cast<std::int16_t>(scaled ? chunk.mArguments[4] : 0),
                    static_cast<std::int16_t>(scaled ? chunk.mArguments[5] : 0)});
            flipped = false;
        } break;
        case Actions::UPDATE:
        {
            currentScene.mActions.emplace_back(Update{});
        } break;
        case Actions::PURGE:
        {
            currentScene.mActions.emplace_back(Purge{});
        } break;
        case Actions::DELAY:
        {
            currentScene.mActions.emplace_back(
                Delay{static_cast<unsigned>(chunk.mArguments[0]) * 10});
        } break;
        default:
            logger.Debug() << "Unhandled action: " << chunk.mAction << "\n";
            break;
        }
    }

    // Push final scene
    PushScene();

    return scenes;
}
}
