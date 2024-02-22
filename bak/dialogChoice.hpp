#pragma once

#include "bak/dialogTarget.hpp"
#include "bak/types.hpp"

#include <ostream>
#include <variant>

namespace BAK {

// Choice's that depend on the game's active state
enum class ActiveStateFlag : std::uint16_t
{
    // Context dependent
    Context    = 0x7530,
    Money      = 0x7531,
    // Tried to buy but can't afford
    CantAfford = 0x7533,
    Chapter    = 0x7537,
    NightTime  = 0x7539,
    DayTime   = 0x753a,
    TimeBetween = 0x753c,
    SkillCheck = 0x753d,
    ItemValue_753e = 0x753e,
    Context_753f = 0x753f,
    // e.g. KeyTarget{1b7767} Repair, Flterchers Post == 4
    Shop       = 0x7542,
    Zone       = 0x7543,
    // 0x7544 - 0x754c - which chapter
    Gambler    = 0x754d // gambler has no money?
};

std::string_view ToString(ActiveStateFlag f);

enum class ChoiceMask : std::uint16_t
{
    NoChoice = 0x0000,
    // Choices as present in conversations with NPCs (1 -> 0xab)
    Conversation = 0x00ab,
    // Keyword choices such as Yes/No, Buy/Haggle (f1 -> 118)
    Query = 0x01ff,
    // Choice based on whether we have seen a note or opened a chest?
    ItemOrChest = 0x19ff, 
    EventFlag = 0x1fff,
    GameState = 0x75ff,
    CustomState = 0x9cff,
    // item that you need is: (c3xx & 0xff) - 0x50
    Inventory = 0xc3ff,
    HaveNote      = 0xc7ff,
    CastSpell     = 0xcbff,
    // Pick a random number (0, 0xfff) % (0xffff & (evPtr + 0x30f8))
    SleepingGlade = 0xcfff,
    ComplexEvent = 0xdfff,
    Unknown      = 0xffff
};

std::string_view ToString(ChoiceMask m);

struct ConversationChoice
{
    std::uint16_t mEventPointer;
    std::string mKeyword;
};

struct QueryChoice
{
    std::uint16_t mQueryIndex;
    std::string mKeyword;
};

struct EventFlagChoice
{
    std::uint16_t mEventPointer;
};

struct GameStateChoice
{
    ActiveStateFlag mState;
};

enum class Scenario : std::uint8_t
{
    MortificationOfTheFlesh = 1,
    Plagued = 2,
    HaveSixSuitsOfArmor = 3,
    AllPartyArmorIsGoodCondition = 4,
    PoisonedDelekhanArmyChests = 5,
    AnyCharacterSansWeapon = 6,
    AlwaysFalse = 7,
    AlwaysFalse2 = 8,
    AnyCharacterHasNegativeCondition = 9,
    AnyCharacterIsUnhealthy = 10,
    AllPartyMembersHaveNapthaMask = 11,
    NormalFoodInArlieChest = 12, // Guess...
    PoisonedFoodInArlieChest = 13 // ?
};

struct CustomStateChoice
{
    Scenario mScenario;
};

struct InventoryChoice
{
    ItemIndex mRequiredItem;
};

struct ComplexEventChoice
{
    std::uint16_t mEventPointer;
    //std::uint8_t mXorMask;
    //std::uint8_t mExpected;
    //std::uint8_t mMustEqualExpected;
    //std::uint8_t mChapterMask;
};

struct NoChoice
{
};

struct CastSpellChoice
{
    unsigned mRequiredSpell;
};

struct HaveNoteChoice 
{
    unsigned mRequiredNote;
};

struct UnknownChoice
{
    ChoiceMask mChoiceCategory;
    std::uint16_t mEventPointer;
};

using Choice = std::variant<
    NoChoice,
    ConversationChoice,
    QueryChoice,
    EventFlagChoice,
    GameStateChoice,
    CustomStateChoice,
    InventoryChoice,
    ComplexEventChoice,
    CastSpellChoice,
    HaveNoteChoice,
    UnknownChoice>;

std::ostream& operator<<(std::ostream&, const Choice&);

Choice CreateChoice(std::uint16_t state);

struct DialogChoice
{
    DialogChoice(
        std::uint16_t state,
        std::uint16_t min,
        std::uint16_t max,
        Target target);

    Choice mChoice;
    std::uint16_t mMin;
    std::uint16_t mMax;
    Target mTarget;
};

std::ostream& operator<<(std::ostream&, const DialogChoice&);

}
