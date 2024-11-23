#include "bak/dialogSources.hpp"

#include "bak/dialog.hpp"

namespace BAK {

KeyTarget DialogSources::GetFairyChestKey(unsigned chest)
{
    return KeyTarget{mFairyChestKey + chest};
}

std::string_view DialogSources::GetItemDescription(unsigned itemIndex)
{
    return DialogStore::Get().GetSnippet(
        GetChoiceResult(mItemDescription, itemIndex)).GetText();
}

std::string_view DialogSources::GetScrollDescription(SpellIndex spellIndex)
{
    return DialogStore::Get().GetSnippet(
        GetChoiceResult(mScrollDescriptions, spellIndex.mValue)).GetText();
}

KeyTarget DialogSources::GetItemUseText(unsigned itemIndex)
{
    // Item use text takes a "context variable" which is
    // the item number.
    return KeyTarget{mItemDescription};
}

KeyTarget DialogSources::GetSpynote()
{
    return KeyTarget{mSpyNoteContents};
}

KeyTarget DialogSources::GetSpellCastDialog(unsigned spell)
{
    return KeyTarget{mDragonsBreath + spell};
}

KeyTarget DialogSources::GetTTMDialogKey(unsigned index)
{
    return KeyTarget{0x186a00 + index};
}

KeyTarget DialogSources::GetChapterStartText(Chapter chapter)
{
    return BAK::KeyTarget{mChapterFullMapScreenText + (chapter.mValue - 1)};
}

Target DialogSources::GetChoiceResult(KeyTarget dialog, unsigned index)
{
    const auto& choices = DialogStore::Get().GetSnippet(dialog).GetChoices();
    const auto it = std::find_if(choices.begin(), choices.end(), [&](const auto& a){
        ASSERT(std::holds_alternative<GameStateChoice>(a.mChoice));
        return a.mMin == index ;} );
    ASSERT(it != choices.end());
    return it->mTarget;
}

}
