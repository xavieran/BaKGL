#pragma once

#include "dialog.hpp"

#include <functional>
#include <iomanip>
#include <type_traits>
#include <variant>

namespace BAK {

class DialogSources
{
public:
    static KeyTarget GetFairyChestKey(unsigned chest)
    {
        return KeyTarget{mFairyChestKey + chest};
    }

    static std::string_view GetItemDescription(unsigned itemIndex)
    {
        return DialogStore::Get().GetSnippet(
            GetChoiceResult(mItemDescription, itemIndex)).GetText();
    }

    static std::string_view GetScrollDescription(SpellIndex spellIndex)
    {
        return DialogStore::Get().GetSnippet(
            GetChoiceResult(mScrollDescriptions, spellIndex.mValue)).GetText();
    }

    static KeyTarget GetItemUseText(unsigned itemIndex)
    {
        // Item use text takes a "context variable" which is
        // the item number.
        return KeyTarget{mItemDescription};
    }

    static KeyTarget GetSpynote()
    {
        return KeyTarget{mSpyNoteContents};
    }

    static KeyTarget GetSpellCastDialog(unsigned spell)
    {
        return KeyTarget{mDragonsBreath + spell};
    }

    static Target GetChoiceResult(KeyTarget dialog, unsigned index)
    {
        const auto& choices = DialogStore::Get().GetSnippet(dialog).GetChoices();
        const auto it = std::find_if(choices.begin(), choices.end(), [&](const auto& a){
            ASSERT(std::holds_alternative<GameStateChoice>(a.mChoice));
            return std::get<GameStateChoice>(a.mChoice).mExpectedValue == index ;} );
        ASSERT(it != choices.end());
        return it->mTarget;
    }

    static constexpr auto mFairyChestKey    = 0x19f0a0;

    static constexpr auto mItemDescription  = KeyTarget{0x1b7741};
    static constexpr auto mItemUseSucessful = KeyTarget{0x1b7742};
    static constexpr auto mItemUseFailure   = KeyTarget{0x1b7743};
    static constexpr auto mHealthPotionDialog = 0x1b7744;
    static constexpr auto mConsumeAntiVenom   = KeyTarget{0x1b776f};
    static constexpr auto mNoSpaceForNewItem  = 0x1b775b;
    static constexpr auto mWarriorCantUseMagiciansItem = KeyTarget{0x1b7745};
    static constexpr auto mMagicianCantUseWarriorsItem = KeyTarget{0x1b7771};
    static constexpr auto mCantUseItemRightNow = 0x1b7746;
    static constexpr auto mCantUseItemDuringCombat = 0x1b7747;
    // Contextual
    static constexpr auto mItemHasNoCharges         = KeyTarget{0x1b776c};
    // Contextual on container type (use to deduce container types?)
    static constexpr auto mContainerHasNoRoomForItem = KeyTarget{0x1b7748};
    static constexpr auto mDropItem = 0x1b7749;
    static constexpr auto mGenericCantUseItem = KeyTarget{0x1b774a};
    static constexpr auto mGenericCharacterClassCantUseItem = 0x1b774b;
    // Though he was fascinated by the @1, he realised that this was neither the time nor the place for him to be toying with it, especially when spying eyes might be watching him.
    static constexpr auto mCantUseItemRightNowSpyingEyes = 0x1b7770;
    static constexpr auto mCantUseItemOnTimirianya = 0x1b7772;
    static constexpr auto mCantGiveItemDuringCombat = 0x1b774c;
    static constexpr auto mCantTakeItemDuringCombat = 0x1b774d;
    static constexpr auto mCantStealItemDuringCombat = 0x1b7769;
    static constexpr auto mCantDiscardOnlyWeapon = KeyTarget{0x1b774e};
    static constexpr auto mCantDropFlamingTorch  = 0x1b775d;
    static constexpr auto mCantRepairItemFurther = KeyTarget{0x1b775e};
    static constexpr auto mCantConsumeMorePotion = KeyTarget{0x1b7760};
    static constexpr auto mLitTorchInNapthaCaverns = KeyTarget{0x1b776e};
    static constexpr auto mInventoryInterfaceTooltips = 0x1b775a;
    static constexpr auto mInventoryInterfaceTooltips2 = 0x1b7751;
    static constexpr auto mPartyFundsTooltip = 0x1b7762;
    // Some of these seem to be unused - dialogs to describe spells that are used
    static constexpr auto mSpellCastDialogs = 0x1b7752;
    static constexpr auto mSpyNoteContents  = 0x1b7753;
    static constexpr auto mUseTimirianyaMap = 0x1b7772;
    static constexpr auto mFailHaggleItemUnavailable = KeyTarget{0x1b7754};
    static constexpr auto mSucceedHaggle             = KeyTarget{0x1b7755};
    static constexpr auto mSellItemDialog            = KeyTarget{0x1b7756};
    static constexpr auto mBuyItemDialog             = KeyTarget{0x1b7757};
    static constexpr auto mCantAffordItem            = KeyTarget{0x1b7758};
    static constexpr auto mShopWontBuyItem           = KeyTarget{0x1b7759};
    static constexpr auto mCantHaggleScroll          = KeyTarget{0x1b775f};
    static constexpr auto mCantBuyTooDrunk           = KeyTarget{0x1b775c};
    static constexpr auto mEmptyPopup                = KeyTarget{0x1b7768};
    static constexpr auto mScrollDescriptions        = KeyTarget{0x1b7761};
    static constexpr auto mRepairShopCost            = KeyTarget{0x1b7763};
    static constexpr auto mRepairShopCantRepairItem  = KeyTarget{0x1b7764};
    static constexpr auto mRepairShopItemDoesntNeedRepair = KeyTarget{0x1b7765};
    static constexpr auto mShopBeginRepairDialog = KeyTarget{0x1b7766};
    static constexpr auto mShopRepairDialogTooltip = KeyTarget{0x1b7767};
    static constexpr auto mLockKnown  = KeyTarget{0x1b776a};
    static constexpr auto mLockDialog = KeyTarget{0x1b776b};

    static constexpr auto mInnDialog  = KeyTarget{0x13d672};

    static constexpr auto mHealDialogTooltips = KeyTarget{0x13d66a};
    static constexpr auto mHealDialogCantHealNotSick = KeyTarget{0x13d66b};
    static constexpr auto mHealDialogCantHealNotSickEnough = KeyTarget{0x13d673};
    static constexpr auto mHealDialogCantAfford = KeyTarget{0x13d66c};
    static constexpr auto mHealDialogCost = KeyTarget{0x13d66d};
    static constexpr auto mHealDialogPostHealing = KeyTarget{0x13d66e};

    static constexpr auto mBlessDialogItemAlreadyBlessed = KeyTarget{0x13d66f};
    static constexpr auto mBlessDialogCost = KeyTarget{0x13d670};
    static constexpr auto mBlessDialogCantBlessItem = KeyTarget{0x13d671};

    static constexpr auto mTempleDialog = KeyTarget{0x13d668};
    static constexpr auto mTeleportDialog = KeyTarget{0x13d663};
    static constexpr auto mTeleportDialogIntro = KeyTarget{0x13d65d};
    static constexpr auto mTeleportDialogCantAfford = KeyTarget{0x13d65e};
    static constexpr auto mTeleportDialogNoDestinations = KeyTarget{0x13d65f};
    static constexpr auto mTeleportDialogPostTeleport = KeyTarget{0x13d660};
    static constexpr auto mTeleportDialogCancel = KeyTarget{0x13d661};
    static constexpr auto mTeleportDialogTeleportedToSameTemple = KeyTarget{0x13d674};
    static constexpr auto mTeleportDialogTeleportBlockedMalacsCrossDest= KeyTarget{0x493fd};
    static constexpr auto mTeleportDialogTeleportBlockedMalacsCrossSource= KeyTarget{0x493fe};

    static constexpr auto mBardingAlreadyDone = KeyTarget{0x47};
    static constexpr auto mBardingBad  = KeyTarget{0x49};
    static constexpr auto mBardingPoor = KeyTarget{0x58};
    static constexpr auto mBardingOkay = KeyTarget{0x59};
    static constexpr auto mBardingGood = KeyTarget{0x5a};
    
    static constexpr auto mWordlockIntro     = KeyTarget{0xc};
    static constexpr auto mCantOpenWorldock  = KeyTarget{0xd};
    static constexpr auto mOpenedWordlock    = KeyTarget{0xe};
    static constexpr auto mTrappedChestExplodes = KeyTarget{0xc0};
    static constexpr auto mOpenUnlockedBox   = KeyTarget{0xc2};
    static constexpr auto mOpenTrappedBox    = KeyTarget{0xbe};
    static constexpr auto mDisarmedTrappedBox = KeyTarget{0xbf};
    static constexpr auto mChooseUnlock      = KeyTarget{0x4f};
    static constexpr auto mOpenExplodedChest = KeyTarget{0x13d};

    static constexpr auto mKeyOpenedLock    = KeyTarget{0x51};
    static constexpr auto mKeyDoesntFit     = KeyTarget{0x52};
    static constexpr auto mLockPicked       = KeyTarget{0x53};
    static constexpr auto mFailedToPickLock = KeyTarget{0x54};
    static constexpr auto mPicklockBroken   = KeyTarget{0x55};
    static constexpr auto mKeyBroken        = KeyTarget{0xf5};

    static constexpr auto mTombNoShovel  = KeyTarget{0x42};
    static constexpr auto mTombNoBody    = KeyTarget{0x43};
    static constexpr auto mTombJustABody = KeyTarget{0x44};

    static constexpr auto mUnknownObject = KeyTarget{0x9a};
    static constexpr auto mBag           = KeyTarget{0x9e};
    static constexpr auto mBody          = KeyTarget{0x4e};
    static constexpr auto mFoodBush      = KeyTarget{0x9f};
    static constexpr auto mCampfire      = KeyTarget{0xa6};
    static constexpr auto mCorn          = KeyTarget{0xaa};
    static constexpr auto mCrystalTree   = KeyTarget{0xb2};
    static constexpr auto mDirtpile      = KeyTarget{0xf};
    static constexpr auto mHealthBush    = KeyTarget{0xa0};
    static constexpr auto mPoisonBush    = KeyTarget{0xa1};
    static constexpr auto mStones        = KeyTarget{0xaf};
    static constexpr auto mScarecrow     = KeyTarget{0xb5};
    static constexpr auto mSiegeEngine   = KeyTarget{0xb7};
    static constexpr auto mStump         = KeyTarget{0xba};
    static constexpr auto mTrappedAnimal = KeyTarget{0xab};
    static constexpr auto mWell          = KeyTarget{0xbc};

    static constexpr auto mGenericScoutedCombat = KeyTarget{0x2f};

    static constexpr auto mCharacterFlavourDialog = BAK::KeyTarget{0x69};

    static constexpr auto mAfterNagoCombatSetKeys = BAK::KeyTarget{0x1cfdf1};
    static constexpr auto mEndOfChapterResetFlags =
        KeyTarget{0x1e8497};
    static constexpr auto mDragonsBreath = 0xc7;
};

}
