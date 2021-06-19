#include "bak/scene.hpp"

#include "com/string.hpp"
#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "xbak/ResourceTag.h"
#include "xbak/TaggedResource.h"

#include <cassert>
#include <functional>
#include <unordered_map>

namespace BAK {


std::ostream& operator<<(std::ostream& os, const Scene& scene)
{
    os << "Scene :: " << scene.mSceneTag << " [";
    auto sep = ' ';
    for (const auto& a : scene.mActions)
    {
        os << sep << a;
        sep = '\n';
    }
    os << " ]\n";
    for (const auto& [key, imagePal] : scene.mImages)
    {
        const auto& [image, palKey] = imagePal;
        os << "K: " << key << " " << image << " pal: " << palKey << "\n";
    }
    for (const auto& [key, pal] : scene.mPalettes)
        os << "K: " << key << " " << pal << "\n";
    return os;
}

std::unordered_map<unsigned, SceneIndex> LoadSceneIndices(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    auto resbuf = fb.Find(TAG_RES);
    auto scrbuf = fb.Find(TAG_SCR);
    auto tagbuf = fb.Find(TAG_TAG);

    if (scrbuf.GetUint8() != 0x02)
    {
        throw std::runtime_error("Script buffer not compressed");
    }

    auto decompressedSize = scrbuf.GetUint32LE();
    auto script = FileBuffer(decompressedSize);
    scrbuf.DecompressLZW(&script);
    
    ResourceTag tags;
    tags.Load(&tagbuf);

    for (const auto& [id, tag] : tags.GetTagMap())
        logger.Debug() << "Id: " << id << " tag: " << tag << "\n";
    
    std::optional<unsigned> currentIndex{};
    std::optional<unsigned> ttmIndex{};

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
                if (!ttmIndex)
                    ttmIndex = std::max(
                        std::max(a, b),
                        std::max(c, d));
            }
                break;
            case AdsActions::END:
            {
                assert(currentIndex);
                assert(ttmIndex);
                auto it = tags.GetTagMap().find(*currentIndex);
                if (it == tags.GetTagMap().end())
                    throw std::runtime_error("Tag not found");
                sceneIndices[*currentIndex] = SceneIndex{
                    it->second,
                    *ttmIndex};
                currentIndex = std::optional<unsigned>{};
                ttmIndex = std::optional<unsigned>{};
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
            case AdsActions::UNKNOWN: [[fallthrough]];
            case AdsActions::UNKNOWN3:
            {
                const auto a = script.GetUint16LE();
                ss << a;
            }
                break;
            case AdsActions::UNKNOWN2: [[fallthrough]];
            case AdsActions::AND: [[fallthrough]];
            case AdsActions::OR: [[fallthrough]];
            case AdsActions::FADE_OUT: [[fallthrough]];
            case AdsActions::END_IF: [[fallthrough]];
            case AdsActions::PLAY_SCENE: [[fallthrough]];
            case AdsActions::PLAY_SCENE2:
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

    auto pageBuffer    = fb.Find(TAG_PAG);
    auto versionBuffer = fb.Find(TAG_VER);
    auto tt3Buffer     = fb.Find(TAG_TT3);
    auto tagBuffer     = fb.Find(TAG_TAG);
    
    const auto pages = pageBuffer.GetUint16LE();
    logger.Debug() << "Pages:" << pages << "\n";

    tt3Buffer.Skip(1);
    FileBuffer decompBuffer = FileBuffer(tt3Buffer.GetUint32LE());
    auto decomped = tt3Buffer.DecompressRLE(&decompBuffer);
    ResourceTag tags;
    tags.Load(&tagBuffer);

    for (const auto& [id, tag] : tags.GetTagMap())
        logger.Debug() << "Id: " << id << " tag: " << tag << "\n";

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
            std::string name;
            if (tags.Find(id, name))
            {
                ss << " Name: " << name;
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
    std::optional<unsigned> paletteSlot = 0;
    std::unordered_map<unsigned, std::string> palettes{};
    std::unordered_map<
        unsigned,
        std::pair<std::string, unsigned>> images{};

    const auto PushScene = [&]{
        currentScene.mPalettes = palettes;
        currentScene.mImages = images;
        currentScene.mActions.emplace_back(DisableClipRegion{});

        const auto& tagMap = tags.GetTagMap();
        auto it = std::find_if(tagMap.begin(), tagMap.end(),
            [&](const auto& it){
                return it.second == currentScene.mSceneTag; });

        if (it != tagMap.end())
            scenes[it->first] = currentScene;
        else
            throw std::runtime_error("Tag not found");
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

            assert(chunk.mResourceName);
            currentScene.mSceneTag = *chunk.mResourceName;
            currentScene.mActions.clear();
            currentScene.mImages.clear();
            currentScene.mPalettes.clear();
            images.clear();
            palettes.clear();
            imageSlot.reset();
            //paletteSlot.reset();
            loadingScene = true;
            break;

        case Actions::SET_CLIP_REGION:
        {
            // Transform this to opengl coords...
            currentScene.mActions.emplace_back(
                ClipRegion{
                    glm::vec2{chunk.mArguments[0], chunk.mArguments[1]},
                    glm::vec2{chunk.mArguments[2], chunk.mArguments[3]}});
        }
            break;

        case Actions::LOAD_PALETTE:
            assert(loadingScene);
            assert(chunk.mResourceName);
            assert(paletteSlot);
            palettes[*paletteSlot] = *chunk.mResourceName;
            break;

        case Actions::LOAD_IMAGE:
        {
            assert(loadingScene);
            assert(chunk.mResourceName);
            assert(imageSlot);
            assert(paletteSlot);
            auto name = *chunk.mResourceName;
            (*(name.end() - 1)) = 'X';
            images[*imageSlot] = std::make_pair(name, *paletteSlot);
        }
            break;
        case Actions::DRAW_SPRITE1: [[fallthrough]];
        case Actions::DRAW_SPRITE_FLIP:
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
        }
            break;
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
