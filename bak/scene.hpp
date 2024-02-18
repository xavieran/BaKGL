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

    TTMIndex GetTTMIndex(const BAK::GameState& gs) const
    {
        const auto chapter = gs.GetChapter().mValue;
        if (mGreaterThan && mLessThan)
        {
            if (chapter >= *mGreaterThan
                && chapter <= *mLessThan)
                return mIf;
            else
            {
                ASSERT(mElse);
                return *mElse;
            }
        }
        else if (mGreaterThan && chapter >= *mGreaterThan)
        {
            return mIf;
        }
        else if (mLessThan && chapter <= *mLessThan)
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

std::ostream& operator<<(std::ostream&, const SceneIndex&);

struct ImageSlot
{
    std::optional<std::string> mImage;
    std::optional<unsigned> mPalette;
};

struct Scene
{
    std::string mSceneTag;
    std::vector<SceneAction> mActions;
    std::unordered_map<unsigned, std::string> mPalettes;
    std::unordered_map<unsigned, std::pair<std::string, unsigned>> mImages;

    std::optional<ClipRegion> mClipRegion;
};

std::ostream& operator<<(std::ostream&, const Scene&);

std::unordered_map<unsigned, SceneIndex> LoadSceneIndices(FileBuffer& fb);
std::unordered_map<unsigned, Scene> LoadScenes(FileBuffer& fb);

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
