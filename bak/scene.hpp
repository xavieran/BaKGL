#pragma once

#include "bak/sceneData.hpp"
#include "bak/types.hpp"

#include <optional>
#include <vector>
#include <unordered_map>
#include <map>

namespace BAK {

class FileBuffer;

using TTMIndex = unsigned;

struct ADSIndex
{
    ADSIndex();

    TTMIndex GetTTMIndex(Chapter chapter) const;

    TTMIndex mIf;
    std::optional<TTMIndex> mElse;
    std::optional<unsigned> mGreaterThan;
    std::optional<unsigned> mLessThan;
};

std::ostream& operator<<(std::ostream&, const ADSIndex&);

struct SceneIndex
{
    std::string mSceneTag;
    ADSIndex mSceneIndex;
};

struct SceneADS
{
    unsigned mInitScene;
    unsigned mDrawScene;
    bool mPlayAllScenes;
};

struct SceneSequence
{
    std::string mName;
    std::vector<SceneADS> mScenes;
};

std::ostream& operator<<(std::ostream&, const SceneIndex&);

struct ImageSlot
{
    std::vector<std::string> mImage;
    std::optional<unsigned> mPalette;
};

using PaletteSlot = unsigned;

struct Scene
{
    std::string mSceneTag;
    std::vector<SceneAction> mActions;
    std::unordered_map<PaletteSlot, std::string> mPalettes;
    std::unordered_map<unsigned, std::pair<std::string, PaletteSlot>> mImages;
    std::unordered_map<PaletteSlot, std::pair<std::string, PaletteSlot>> mScreens;

    std::optional<ClipRegion> mClipRegion;
};

std::ostream& operator<<(std::ostream&, const Scene&);

struct DynamicScene
{
    std::map<unsigned, unsigned> mScenes;
    std::vector<SceneAction> mActions;
};

std::unordered_map<unsigned, std::vector<SceneSequence>> LoadSceneSequences(FileBuffer& fb);
std::unordered_map<unsigned, SceneIndex> LoadSceneIndices(FileBuffer& fb);
std::unordered_map<unsigned, Scene> LoadScenes(FileBuffer& fb);
std::vector<SceneAction> LoadDynamicScenes(FileBuffer& fb);

FileBuffer DecompressTTM(FileBuffer& fb);

}
