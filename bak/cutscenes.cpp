#include "bak/cutscenes.hpp"

#include <sstream>

namespace BAK {

std::vector<CutsceneAction> CutsceneList::GetStartScene(Chapter chapter)
{
    std::vector<CutsceneAction> actions{};

    std::stringstream ss{};
    ss << "CHAPTER" << chapter.mValue;
    actions.emplace_back(TTMScene{ss.str() + ".ADS", ss.str() + ".TTM"});

    ss = {};
    ss << "C" << chapter.mValue << "1";
    actions.emplace_back(BookChapter{ss.str() + ".BOK"});

    ss = {};
    ss << "C" << chapter.mValue << "1";
    actions.emplace_back(TTMScene{ss.str() + ".ADS", ss.str() + ".TTM"});

    return actions;
}

std::vector<CutsceneAction> CutsceneList::GetFinishScene(Chapter chapter)
{
    std::vector<CutsceneAction> actions{};

    std::stringstream ss{};
    if (chapter == Chapter{2}
        || chapter == Chapter{4}
        || chapter == Chapter{6}
        || chapter == Chapter{7}
        || chapter == Chapter{8})
    {
    }
    else
    {
        ss << "C" << chapter.mValue << "2";
        actions.emplace_back(BookChapter{ss.str() + ".BOK"});
    }

    ss = {};
    ss << "C" << chapter.mValue << "2";
    actions.emplace_back(TTMScene{ss.str() + ".ADS", ss.str() + ".TTM"});

    // Actually this is after the final battle
    if (chapter == Chapter{9})
    {
        ss = {};
        ss << "C" << chapter.mValue << "3";
        actions.emplace_back(TTMScene{ss.str() + ".ADS", ss.str() + ".TTM"});
    }

    return actions;

}
}
