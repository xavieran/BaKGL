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

    static KeyTarget GetItemDescription()
    {
        return KeyTarget{mItemDescription};
    }

    static KeyTarget GetScrollDescription()
    {
        return KeyTarget{mScrollDescriptions};
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

    static constexpr auto mFairyChestKey    = 0x19f0a0;

    static constexpr auto mItemDescription  = 0x1b7741;
    static constexpr auto mItemUseSucessful = 0x1b7742;
    static constexpr auto mItemUseFailure   = 0x1b7743;
    static constexpr auto mHealthPotionDialog = 0x1b7744;
    static constexpr auto mConsumeAntiVenom   = 0x1b776f;
    static constexpr auto mNoSpaceForNewItem  = 0x1b775b;
    static constexpr auto mCantConsumeAlcohol = 0x1b775c;
    static constexpr auto mWarriorCantUseMagiciansItem = 0x1b7745;
    static constexpr auto mMagicianCantUseWarriorsItem = 0x1b7771;
    static constexpr auto mCantUseItemRightNow = 0x1b7746;
    static constexpr auto mCantUseItemDuringCombat = 0x1b7747;
    // Contextual
    static constexpr auto mItemHasNoCharges         = 0x1b776c;
    // Contextual on container type (use to deduce container types?)
    static constexpr auto mContainerHasNoRoomForItem = KeyTarget{0x1b7748};
    static constexpr auto mDropItem = 0x1b7749;
    static constexpr auto mGenericCantUseItem = 0x1b774a;
    static constexpr auto mGenericCharacterClassCantUseItem = 0x1b774b;
    // Though he was fascinated by the @1, he realised that this was neither the time nor the place for him to be toying with it, especially when spying eyes might be watching him.
    static constexpr auto mCantUseItemRightNowSpyingEyes = 0x1b7770;
    static constexpr auto mCantUseItemOnTimirianya = 0x1b7772;
    static constexpr auto mCantGiveItemDuringCombat = 0x1b774c;
    static constexpr auto mCantTakeItemDuringCombat = 0x1b774d;
    static constexpr auto mCantStealItemDuringCombat = 0x1b7769;
    static constexpr auto mCantDiscardOnlyWeapon = 0x1b774e;
    static constexpr auto mCantDropFlamingTorch  = 0x1b775d;
    static constexpr auto mCantRepairItem        = 0x1b775e;
    static constexpr auto mCantHaggleScroll      = 0x1b775f;
    static constexpr auto mCantConsumeMorePoition = 0x1b7760;
    static constexpr auto mLitTorchInNapthaCaverns = 0x1b776e;
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
    static constexpr auto mEmptyPopup                = KeyTarget{0x1b7768};
    static constexpr auto mScrollDescriptions        = KeyTarget{0x1b7761};
    static constexpr auto mRepairShopDialog          = KeyTarget{0x1b7763};
    static constexpr auto mRepairShopCantRepairItem  = KeyTarget{0x1b7764};
    static constexpr auto mRepairShopItemDoesntNeedRepair = 0x1b7765;
    static constexpr auto mShopBeginRepairDialog = 0x1b7766;
    static constexpr auto mShopRepairDialogTooltip = 0x1b7767;
    static constexpr auto mLockKnownNoPicklocks = 0x1b776a;
    static constexpr auto mLockDialog = 0x1b776b;

    static constexpr auto mInnDialog  = KeyTarget{0x13d672};

    static constexpr auto mHealDialogTooltips = 0x13d66a;
    static constexpr auto mHealDialogCantHealNotSick = 0x13d66b;
    static constexpr auto mHealDialogCantHealNotSickEnough = 0x13d673;
    static constexpr auto mHealDialogCantAfford = 0x13d66c;
    static constexpr auto mHealDialogCost = 0x13d66d;
    static constexpr auto mHealDialogPostHealing = 0x13d66e;

    static constexpr auto mBlessDialogItemAlreadyBlessed = 0x13d66f;
    static constexpr auto mBlessDialogCost = 0x13d670;
    static constexpr auto mBlessDialogCantBlessItem = 0x13d671;

    static constexpr auto mTempleDialog = KeyTarget{0x13d668};
    static constexpr auto mTeleportDialog = KeyTarget{0x13d663};
    static constexpr auto mTeleportDialogIntro = KeyTarget{0x13d65d};
    static constexpr auto mTeleportDialogCantAfford = 0x13d65e;
    static constexpr auto mTeleportDialogNoDestinations = 0x13d65f;
    static constexpr auto mTeleportDialogPostTeleport = 0x13d660;
    static constexpr auto mTeleportDialogCancel = 0x13d661;
    static constexpr auto mTeleportDialogTeleportedToSameTemple = 0x13d674;

    static constexpr auto mBardingAlreadyDone = KeyTarget{0x47};
    static constexpr auto mBardingBad  = KeyTarget{0x49};
    static constexpr auto mBardingPoor = KeyTarget{0x58};
    static constexpr auto mBardingOkay = KeyTarget{0x59};
    static constexpr auto mBardingGood = KeyTarget{0x5a};

};

}
