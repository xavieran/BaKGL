#include "bak/dialogChoice.hpp"

#include "bak/spells.hpp"

#include "com/logger.hpp"

namespace BAK {

std::string ToString(ActiveStateFlag f)
{
    switch (f)
    {
    case ActiveStateFlag::Context: return "Context";
    case ActiveStateFlag::Money: return "Money";
    case ActiveStateFlag::CantAfford: return "CantAfford";
    case ActiveStateFlag::Chapter: return "Chapter";
    case ActiveStateFlag::NightTime: return "NightTime";
    case ActiveStateFlag::DayTime: return "DayTime";
    case ActiveStateFlag::TimeBetween: return "TimeBetween";
    case ActiveStateFlag::SkillCheck: return "SkillCheck";
    case ActiveStateFlag::ItemValue_753e: return "ItemValue_753e";
    case ActiveStateFlag::Context_753f: return "Context_753f";
    case ActiveStateFlag::Shop: return "Shop";
    case ActiveStateFlag::Gambler: return "Gambler";
    default: return "UnknownActiveStateFlag[" + std::to_string(static_cast<unsigned>(f)) + "]";
    }
}

std::string_view ToString(ChoiceMask m)
{
    switch (m)
    {
    case ChoiceMask::NoChoice: return "NoChoice";
    case ChoiceMask::Conversation: return "Conversation";
    case ChoiceMask::Query: return "Query";
    case ChoiceMask::EventFlag: return "EventFlag";
    case ChoiceMask::GameState: return "GameState";
    case ChoiceMask::CustomState: return "CustomState";
    case ChoiceMask::Inventory: return "Inventory";
    case ChoiceMask::HaveNote: return "HaveNote";
    case ChoiceMask::CastSpell: return "CastSpell";
    case ChoiceMask::Random: return "Random";
    case ChoiceMask::ComplexEvent: return "ComplexEvent";
    default: return "UnknownChoiceMask";
    };
}

std::ostream& operator<<(std::ostream& os, const NoChoice& c)
{
    os << ToString(ChoiceMask::NoChoice);
    return os;
}

std::ostream& operator<<(std::ostream& os, const ConversationChoice& c)
{
    os << ToString(ChoiceMask::Conversation) << " " << std::hex
        << c.mEventPointer << " " << c.mKeyword << std::dec;
    return os;
}

std::ostream& operator<<(std::ostream& os, const QueryChoice& c)
{
    os << ToString(ChoiceMask::Query) << " " << std::hex
        << c.mQueryIndex << std::dec << c.mKeyword;
    return os;
}

std::ostream& operator<<(std::ostream& os, const EventFlagChoice& c)
{
    os << ToString(ChoiceMask::EventFlag) << "(" << std::hex 
        << c.mEventPointer << std::dec << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const GameStateChoice& c)
{
    os << ToString(ChoiceMask::GameState) << "(" << ToString(c.mState) << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, Scenario s)
{
    switch (s)
    {
        case Scenario::MortificationOfTheFlesh: os << "MortificationOfTheFlesh"; break;
        case Scenario::Plagued: os << "Plagued"; break;
        case Scenario::HaveSixSuitsOfArmor: os << "HaveSixSuitsOfArmor"; break;
        case Scenario::AllPartyArmorIsGoodCondition: os << "AllPartyArmorIsGoodCondition"; break;
        case Scenario::PoisonedDelekhanArmyChests: os << "PoisonedDelekhanArmyChests"; break;
        case Scenario::AnyCharacterSansWeapon: os << "AnyCharacterSansWeapon"; break;
        case Scenario::AnyCharacterHasNegativeCondition: os << "AnyCharacterHasNegativeCondition"; break;
        case Scenario::AnyCharacterIsUnhealthy: os << "AnyCharacterIsUnhealthy"; break;
        case Scenario::AllPartyMembersHaveNapthaMask: os << "AllPartyMembersHaveNapthaMask"; break;
        case Scenario::NormalFoodInArlieChest: os << "NormalFoodInArlieChest"; break;
        case Scenario::PoisonedFoodInArlieChest: os << "PoisonedFoodInArlieChest"; break;
        default: os << "Unknown(" << static_cast<unsigned>(s) << ")";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const CustomStateChoice& c)
{
    os << ToString(ChoiceMask::CustomState) << "( scenario: " << c.mScenario << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const InventoryChoice& c)
{
    os << ToString(ChoiceMask::Inventory) << "( item: " << c.mRequiredItem << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ComplexEventChoice& c)
{
    os << ToString(ChoiceMask::ComplexEvent) << " " << std::hex << 
        c.mEventPointer << std::dec;
    return os;
}

std::ostream& operator<<(std::ostream& os, const CastSpellChoice& c)
{
    os << ToString(ChoiceMask::CastSpell) << " " << c.mRequiredSpell << "\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const HaveNoteChoice& c)
{
    os << ToString(ChoiceMask::HaveNote) << "(" << c.mRequiredNote << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const RandomChoice& c)
{
    os << ToString(ChoiceMask::Random) << "(" << c.mRange << ")";
    return os;
}
std::ostream& operator<<(std::ostream& os, const UnknownChoice& c)
{
    os << ToString(c.mChoiceCategory) << " " << std::hex << 
        c.mEventPointer << std::dec << "\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Choice& c)
{
    std::visit(
        [&os](const auto& choice)
        {
            os << choice;
        },
        c);
    return os;
}

std::ostream& operator<<(std::ostream& os, const DialogChoice& d)
{
    os << "DialogChoice{" << d.mChoice << " range: ["
        << d.mMin << ", " << d.mMax << "] -> " << std::hex << d.mTarget << std::dec << "}";
    return os;
}

ChoiceMask CategoriseChoice(std::uint16_t state)
{
    const auto CheckMask = [](const auto s, const ChoiceMask m)
    {
        return s <= static_cast<std::uint16_t>(m);
    };

    for (const auto c : {
        ChoiceMask::NoChoice,
        ChoiceMask::Conversation,
        ChoiceMask::Query,
        ChoiceMask::EventFlag,
        ChoiceMask::GameState,
        ChoiceMask::CustomState,
        ChoiceMask::Inventory,
        ChoiceMask::HaveNote,
        ChoiceMask::CastSpell,
        ChoiceMask::Random,
        ChoiceMask::ComplexEvent})
    {
        if (CheckMask(state, c))
            return c;
    }

    return ChoiceMask::Unknown;
}

Choice CreateChoice(std::uint16_t state)
{
    const auto mask = CategoriseChoice(state);
    switch (mask)
    {
    case ChoiceMask::NoChoice:
        return NoChoice{};
    case ChoiceMask::Conversation:
        return ConversationChoice{state, ""};
    case ChoiceMask::Query:
        return QueryChoice{state, ""};
    case ChoiceMask::EventFlag:
        return EventFlagChoice{state};
    case ChoiceMask::GameState:
        return GameStateChoice{static_cast<ActiveStateFlag>(state)};
    case ChoiceMask::CustomState:
        return CustomStateChoice{
            static_cast<Scenario>(state & ~0x9c40)};
    case ChoiceMask::Inventory:
        return InventoryChoice{
            static_cast<ItemIndex>(0xffff & (state + 0x3cb0))};
    case ChoiceMask::ComplexEvent:
        return ComplexEventChoice{state};
    case ChoiceMask::CastSpell:
        return CastSpellChoice{static_cast<unsigned>(state) - 0xcb21};
    case ChoiceMask::HaveNote:
        return HaveNoteChoice{(static_cast<unsigned>(state) + 0x38c8) & 0xffff};
    case ChoiceMask::Random:
        return RandomChoice{(static_cast<unsigned>(state) + 0x30f8) & 0xffff};
    default:
        return UnknownChoice{mask, state};
    }
}

DialogChoice::DialogChoice(
    std::uint16_t state,
    std::uint16_t min,
    std::uint16_t max,
    Target target)
:
    mChoice{CreateChoice(state)},
    mMin{min},
    mMax{max},
    mTarget{target}
{
}

}
