#include "bak/scene/ads.hpp"

#include "bak/scene/scene.hpp"
#include "bak/scene/sceneData.hpp"
#include "bak/file/fileBuffer.hpp"
#include "bak/tags.hpp"
#include "bak/dataTags.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"
#include "com/visit.hpp"

#include <cstdint>
#include <ostream>
#include <vector>

namespace BAK::ADS {

namespace {

struct RawOp
{
    AdsActions mOp;
    std::vector<std::uint16_t> mArgs;
};

unsigned OperandCount(AdsActions op)
{
    switch (op)
    {
        case AdsActions::IF_NOT_PLAYED:
        case AdsActions::IF_NOT_PLAYED_ELSE:
        case AdsActions::IF_PLAYED_ELSE:
            return 2;
        case AdsActions::RESTART_SCRIPT:
        case AdsActions::START_SCRIPT:
            return 4;
        case AdsActions::STOP_SCRIPT:
            return 3;
        case AdsActions::IF_CHAP_GTE:
        case AdsActions::IF_CHAP_LTE:
        case AdsActions::STOP_SCENE:
            return 1;
        default:
            return 0;
    }
}

bool IsCondition(AdsActions op)
{
    switch (op)
    {
        case AdsActions::IF_NOT_PLAYED:
        case AdsActions::IF_NOT_PLAYED_ELSE:
        case AdsActions::IF_PLAYED_ELSE:
        case AdsActions::IF_CHAP_GTE:
        case AdsActions::IF_CHAP_LTE:
            return true;
        default:
            return false;
    }
}

bool IsAction(AdsActions op)
{
    switch (op)
    {
        case AdsActions::START_SCRIPT:
        case AdsActions::RESTART_SCRIPT:
        case AdsActions::STOP_SCRIPT:
            return true;
        default:
            return false;
    }
}

Condition MakeCondition(const RawOp& op)
{
    switch (op.mOp)
    {
        case AdsActions::IF_NOT_PLAYED:
        case AdsActions::IF_NOT_PLAYED_ELSE:
            return NotStarted{ScriptIndex{op.mArgs[1]}};
        case AdsActions::IF_PLAYED_ELSE:
            return Finished{ScriptIndex{op.mArgs[1]}};
        case AdsActions::IF_CHAP_GTE:
            return ChapterGTE{op.mArgs[0]};
        case AdsActions::IF_CHAP_LTE:
            return ChapterLTE{op.mArgs[0]};
        default:
            ASSERT(false);
            return NotStarted{ScriptIndex{0}};
    }
}

Action MakeAction(const RawOp& op)
{
    switch (op.mOp)
    {
        case AdsActions::START_SCRIPT:
        // Only used by LAMUT, effectively same as Start..
        case AdsActions::RESTART_SCRIPT:
            return StartScript{ScriptIndex{op.mArgs[1]}};
        case AdsActions::STOP_SCRIPT:
            return StopScript{ScriptIndex{op.mArgs[1]}};
        default:
            ASSERT(false);
            return StartScript{ScriptIndex{0}};
    }
}

class Parser
{
public:
    Parser(const std::vector<RawOp>& ops)
    :
        mOps{ops},
        mPos{0}
    {}

    std::vector<ActionBlock> ParseBlocks()
    {
        std::vector<ActionBlock> blocks;
        while (mPos < mOps.size())
        {
            blocks.emplace_back(ParseBlock());
        }
        return blocks;
    }

private:
    const RawOp& Peek() const { return mOps[mPos]; }
    bool AtEnd() const { return mPos >= mOps.size(); }

    std::vector<Condition> ParseCondition()
    {
        std::vector<Condition> condition;
        condition.emplace_back(MakeCondition(mOps[mPos++]));
        while (!AtEnd()
            && (Peek().mOp == AdsActions::AND || Peek().mOp == AdsActions::OR))
        {
            // there's no OR in BaK files
            ASSERT(Peek().mOp == AdsActions::AND);
            mPos++;
            condition.emplace_back(MakeCondition(mOps[mPos++]));
        }
        return condition;
    }

    void ParseBody(std::vector<Action>& out)
    {
        while (!AtEnd())
        {
            const auto op = Peek().mOp;
            if (IsAction(op))
            {
                out.emplace_back(MakeAction(mOps[mPos++]));
            }
            else if (IsCondition(op))
            {
                ParseCondition();
                ParseBody(out);
                if (!AtEnd() && Peek().mOp == AdsActions::ELSE)
                {
                    mPos++;
                    ParseBody(out);
                }
                if (!AtEnd()
                    && (Peek().mOp == AdsActions::END_IF
                        || Peek().mOp == AdsActions::END_IF_ELSE))
                {
                    mPos++;
                }
            }
            else
            {
                // ELSE, END_IF, END_IF_ELSE
                break;
            }
        }
    }

    ActionBlock ParseBlock()
    {
        ASSERT(!AtEnd() && IsCondition(Peek().mOp));
        ActionBlock block;
        block.mConditions = ParseCondition();
        ParseBody(block.mThen);
        if (!AtEnd() && Peek().mOp == AdsActions::ELSE)
        {
            mPos++;
            ParseBody(block.mElse);
        }
        if (!AtEnd()
            && (Peek().mOp == AdsActions::END_IF
                || Peek().mOp == AdsActions::END_IF_ELSE))
        {
            mPos++;
        }
        return block;
    }

    const std::vector<RawOp>& mOps;
    std::size_t mPos;
};

}

std::ostream& operator<<(std::ostream& os, const Action& a)
{
    std::visit(overloaded{
        [&](const StartScript& s){ os << "Start(" << s.mScriptIndex << ")"; },
        [&](const StopScript& s){ os << "Stop(" << s.mScriptIndex << ")"; }},
        a);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Condition& c)
{
    std::visit(overloaded{
        [&](const NotStarted& x){ os << "NotStarted(" << x.mScriptIndex << ")"; },
        [&](const Finished& x){ os << "Finished(" << x.mScriptIndex << ")"; },
        [&](const ChapterGTE& x){ os << "ChapterGTE(" << x.mChapter << ")"; },
        [&](const ChapterLTE& x){ os << "ChapterLTE(" << x.mChapter << ")"; }},
        c);
    return os;
}

std::ostream& operator<<(std::ostream& os, const ActionBlock& b)
{
    os << "if ";
    for (unsigned i = 0; i < b.mConditions.size(); i++)
    {
        if (i) os << " and ";
        os << b.mConditions[i];
    }
    os << "\n";
    for (const auto& a : b.mThen)
    {
        os << "    " << a << "\n";
    }
    if (!b.mElse.empty())
    {
        os << "else\n";
        for (const auto& a : b.mElse)
        {
            os << "    " << a << "\n";
        }
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Scene& s)
{
    os << "Scene " << s.mSceneIndex << " (" << s.mTag << "):\n";
    for (const auto& block : s.mBlocks)
    {
        os << "  " << block;
    }
    return os;
}


std::ostream& operator<<(std::ostream& os, const Ads& ads)
{
    for (const auto& scene : ads.mScenes)
    {
        os << scene;
    }
    return os;
}

Ads LoadAds(FileBuffer& fb)
{
    auto tagBuffer = fb.Find(DataTag::TAG);
    Tags tags{};
    tags.Load(tagBuffer);

    auto buffer = DecompressSCR(fb);

    Ads ads;
    while (!buffer.AtEnd())
    {
        const unsigned index = buffer.GetUint16LE();

        std::vector<RawOp> ops;
        for (;;)
        {
            const auto op = static_cast<AdsActions>(buffer.GetUint16LE());
            if (op == AdsActions::END)
            {
                break;
            }
            RawOp raw{op, {}};
            const auto count = OperandCount(op);
            for (unsigned i = 0; i < count; i++)
            {
                raw.mArgs.emplace_back(buffer.GetUint16LE());
            }
            ops.emplace_back(std::move(raw));
        }

        Parser parser{ops};
        Scene scene{};
        scene.mSceneIndex = SceneIndex{index};
        scene.mTag = tags.GetTag(Tag{index}).value_or("");
        scene.mBlocks = parser.ParseBlocks();
        ads.mScenes.emplace_back(std::move(scene));
    }
    return ads;
}

}
