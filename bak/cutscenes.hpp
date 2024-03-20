#pragma once

#include "bak/types.hpp"

#include <string>
#include <variant>
#include <vector>

namespace BAK {

struct TTMScene
{
    std::string mAdsFile;
    std::string mTTMFile;
};

struct BookChapter
{
    std::string mBookFile;
};


using CutsceneAction = std::variant<TTMScene, BookChapter>;

class CutsceneList
{
public:
    static std::vector<CutsceneAction> GetStartScene(Chapter);
    static std::vector<CutsceneAction> GetFinishScene(Chapter);
};

}
