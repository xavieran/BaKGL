#pragma once

#include "bak/gameState.hpp"
#include "bak/sceneData.hpp"
#include "bak/types.hpp"

#include "com/assert.hpp"

#include "bak/fileBufferFactory.hpp"

#include <optional>
#include <vector>
#include <unordered_map>

namespace BAK {

using TTMIndex = unsigned;

struct ADSIndex
{
    ADSIndex()
    :
        mIf{0},
        mElse{},
        mGreaterThan{},
        mLessThan{}
    {}

    TTMIndex GetTTMIndex(Chapter chapter) const
    {
        if (mGreaterThan && mLessThan)
        {
            if (chapter.mValue >= *mGreaterThan
                && chapter.mValue <= *mLessThan)
                return mIf;
            else
            {
                ASSERT(mElse);
                return *mElse;
            }
        }
        else if (mGreaterThan && chapter.mValue >= *mGreaterThan)
        {
            return mIf;
        }
        else if (mLessThan && chapter.mValue <= *mLessThan)
        {
            return mIf;
        }
        else if (!mGreaterThan && !mLessThan)
        {
            return mIf;
        }
        else
        {
            ASSERT(mElse);
            return *mElse;
        }
    }

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
};

struct SceneSequence
{
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
    std::string mSceneTag;
    std::vector<SceneAction> mActions;
};

std::unordered_map<unsigned, std::vector<SceneSequence>> LoadSceneSequences(FileBuffer& fb);
std::unordered_map<unsigned, SceneIndex> LoadSceneIndices(FileBuffer& fb);
std::unordered_map<unsigned, Scene> LoadScenes(FileBuffer& fb);
std::unordered_map<unsigned, DynamicScene> LoadDynamicScenes(FileBuffer& fb);

// Helper during loading
struct SceneChunk
{
    SceneChunk(
        Actions action,
        std::optional<std::string> resourceName,
        std::vector<std::int16_t> arguments)
    :
        mAction{action},
        mResourceName{resourceName},
        mArguments{arguments}
    {}

    Actions mAction;
    std::optional<std::string> mResourceName;
    std::vector<std::int16_t> mArguments;
};


}
