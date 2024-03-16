#include "gui/bookPlayer.hpp"

#include "bak/fileBufferFactory.hpp"

namespace Gui {

BookPlayer::BookPlayer(
    Graphics::SpriteManager& spriteManager,
    const Font& font,
    const Backgrounds& background)
:
    mSpriteManager{spriteManager},
    mSpriteSheet{spriteManager.AddTemporarySpriteSheet()},
    mFont{font},
    mTextures{},
    mBackground(
       ImageTag{},
       background.GetSpriteSheet(),
       background.GetScreen("BOOK.SCX"),
       {0, 0},
       {320, 200},
       true)
{
    auto bookTextures = Graphics::TextureStore{};
    BAK::TextureFactory::AddToTextureStore(
        mTextures, "BOOK.BMX", "BOOK.PAL");
    spriteManager.GetSpriteSheet(mSpriteSheet->mSpriteSheet).LoadTexturesGL(mTextures);
}

void BookPlayer::PlayBook(std::string book)
{
    auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(book);
    mBook = BAK::LoadBook(fb);
    mCurrentPage = 0;
    //mText = mBook->mPages[mCurrentPage].mParagraphs;
    RenderPage(mBook->mPages[mCurrentPage]);
}

void BookPlayer::AdvancePage()
{
    mCurrentPage++;
    if (mCurrentPage == mBook->mPages.size())
    {
        mCurrentPage = 0;
    }
    RenderPage(mBook->mPages[mCurrentPage]);
}

void BookPlayer::RenderPage(const BAK::Page& page)
{
    Logging::LogDebug(__FUNCTION__) << page << "\n";

    mBackground.ClearChildren();
    for (const auto& image : page.mImages)
    {
        auto pos = image.mPos / 2;
        auto dims = mTextures.GetTexture(image.mImage).GetDims() / 2;
        mImages.emplace_back(Widget{
            ImageTag{},
            mSpriteSheet->mSpriteSheet,
            Graphics::TextureIndex{image.mImage},
            pos,
            dims,
            false});
        if (image.mMirroring == BAK::Mirroring::HorizontalAndVertical)
        {
            mImages.back().SetPosition(pos + dims);
            mImages.back().SetDimensions(-dims);
        }
    }

    for (auto& w : mImages)
    {
        mBackground.AddChildBack(&w);
    }
}

Widget* BookPlayer::GetBackground()
{
    return &mBackground;
}
}
