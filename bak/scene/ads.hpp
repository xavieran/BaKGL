#pragma once

#include "com/strongType.hpp"

#include <iosfwd>
#include <string>
#include <variant>
#include <vector>

namespace BAK {
class FileBuffer;
}

namespace BAK::ADS {

using SceneIndex = StrongType<unsigned, struct SceneIndexTag>;
using ScriptIndex = StrongType<unsigned, struct ScriptIndexTag>;

struct StartScript
{
    ScriptIndex mScriptIndex;
};

struct StopScript
{
    ScriptIndex mScriptIndex;
};

using Action = std::variant<
    StartScript,
    StopScript>;

struct NotStarted
{
    ScriptIndex mScriptIndex;
};

struct Finished
{
    ScriptIndex mScriptIndex;
};

struct ChapterGTE
{
    unsigned mChapter;
};

struct ChapterLTE
{
    unsigned mChapter;
};

using Condition = std::variant<
    NotStarted,
    Finished,
    ChapterGTE,
    ChapterLTE>;

struct ActionBlock
{
    std::vector<Condition> mConditions;
    std::vector<Action> mThen;
    std::vector<Action> mElse;
};

struct Scene
{
    unsigned mSceneIndex;
    std::string mTag;
    std::vector<ActionBlock> mBlocks;
};

struct Ads
{
    std::vector<Scene> mScenes;
};

std::ostream& operator<<(std::ostream&, const Action&);
std::ostream& operator<<(std::ostream&, const Condition&);
std::ostream& operator<<(std::ostream&, const ActionBlock&);
std::ostream& operator<<(std::ostream&, const Scene&);
std::ostream& operator<<(std::ostream&, const Ads&);

Ads LoadAds(FileBuffer& fb);

}
