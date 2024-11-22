#include "gui/contents.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include "bak/cutscenes.hpp"
#include "bak/textureFactory.hpp"
#include "bak/types.hpp"

#include <glm/glm.hpp>

namespace Gui {

ContentsScreen::ContentsScreen(
    IGuiManager& guiManager,
    Graphics::SpriteManager& spriteManager,
    const Backgrounds& backgrounds,
    const Font& font,
    LeaveContentsFn&& leaveContentsFn)
:
    Widget{
        RectTag{},
        glm::vec2{0, 0},
        glm::vec2{320, 200},
        Color::black,
        false 
    },
    mGuiManager{guiManager},
    mSpriteSheet{spriteManager.AddTemporarySpriteSheet()},
    mFont{font},
    mBackgrounds{backgrounds},
    mLayout{sLayoutFile},
    mLeaveContentsFn{std::move(leaveContentsFn)},
    mFrame{
        ImageTag{},
        backgrounds.GetSpriteSheet(),
        backgrounds.GetScreen(sBackground),
        glm::vec2{0},
        GetPositionInfo().mDimensions,
        true
    },
    mChapterButtons{},
    mExit{
        mLayout.GetWidgetLocation(sExit),
        mLayout.GetWidgetDimensions(sExit),
        mFont,
        "#Exit",
        [this]{ std::invoke(mLeaveContentsFn); }
    }
{
    auto screen = Graphics::TextureStore{};
    BAK::TextureFactory::AddScreenToTextureStore(screen, sUnlockedChapters, "CONTENTS.PAL");
    const auto source = screen.GetTexture(0);

    auto textures = Graphics::TextureStore{};
    for (unsigned i = 0; i < 9; i++)
    {
        textures.AddTexture(
            source.GetRegion(
                // Why this offset... ?
                mLayout.GetWidgetLocation(i) + glm::vec2{0, 17},
                mLayout.GetWidgetDimensions(i)));
    }
    spriteManager.GetSpriteSheet(mSpriteSheet->mSpriteSheet).LoadTexturesGL(textures);
    AddChildren();
}

void ContentsScreen::AddChildren()
{
    ClearChildren();

    AddChildBack(&mFrame);

    mChapterButtons.clear();
    mChapterButtons.reserve(9);
    for (unsigned i = 0 ; i < 9; i++)
    {
        mChapterButtons.emplace_back(
            [this, i]()
            {
                PlayChapter(BAK::Chapter{i + 1});
            },
            ImageTag{},
            mSpriteSheet->mSpriteSheet,
            Graphics::TextureIndex{8 - i},
            mLayout.GetWidgetLocation(i),
            mLayout.GetWidgetDimensions(i),
            false);
        AddChildBack(&mChapterButtons.back());
    }

    mExit.SetPosition(mLayout.GetWidgetLocation(sExit));

    AddChildBack(&mExit);
}

void ContentsScreen::PlayChapter(BAK::Chapter chapter)
{
    auto start = BAK::CutsceneList::GetStartScene(chapter);
    auto finish = BAK::CutsceneList::GetFinishScene(chapter);
    start.insert(start.end(), finish.begin(), finish.end());
    mGuiManager.PlayCutscene(start, []{});
}

}
