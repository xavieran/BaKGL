#include "bak/party.hpp"

#include "bak/itemNumbers.hpp"
#include "bak/money.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

namespace BAK {

std::size_t Party::GetNumCharacters() const
{
    return mActiveCharacters.size();
}

const Character& Party::GetCharacter(CharIndex i) const
{
    //ASSERT(mCharacters.size() == sMaxCharacters);
    ASSERT(i.mValue < mCharacters.size());
    return mCharacters[i.mValue];
}

Character& Party::GetCharacter(CharIndex i)
{
    //ASSERT(mCharacters.size() == sMaxCharacters);
    ASSERT(i.mValue < mCharacters.size());
    return mCharacters[i.mValue];
}

std::optional<ActiveCharIndex> Party::FindActiveCharacter(CharIndex index) const
{
    const auto it = std::find(mActiveCharacters.begin(), mActiveCharacters.end(), index);
    if (it != mActiveCharacters.end())
        return ActiveCharIndex{
            static_cast<unsigned>(
                std::distance(mActiveCharacters.begin(), it))};
    else
        return std::optional<ActiveCharIndex>{};
}

std::optional<ActiveCharIndex> Party::GetSpellcaster() const
{
    for (unsigned i = 0; i < mActiveCharacters.size(); i++)
    {
        if (GetCharacter(ActiveCharIndex{i}).IsSpellcaster())
        {
            return ActiveCharIndex{i};
        }
    }
    return std::nullopt;
}

const Character& Party::GetCharacter(ActiveCharIndex i) const
{
    ASSERT(mActiveCharacters.size() <= sMaxActiveCharacters);
    return GetCharacter(mActiveCharacters[i.mValue]);
}

Character& Party::GetCharacter(ActiveCharIndex i)
{
    ASSERT(mActiveCharacters.size() <= sMaxActiveCharacters);
    return GetCharacter(mActiveCharacters[i.mValue]);
}

void Party::SetActiveCharacters(const std::vector<CharIndex>& characters)
{
    mActiveCharacters = characters;
}

const KeyContainer& Party::GetKeys() const
{
    return mKeys;
}

KeyContainer& Party::GetKeys()
{
    return mKeys;
}

Royals Party::GetGold() const
{
    return mGold;
}

bool Party::HaveItem(ItemIndex itemIndex) const
{
    const auto item = InventoryItemFactory::MakeItem(
        itemIndex,
        1);

    for (const auto& character : mActiveCharacters)
        if (GetCharacter(character).GetInventory().HaveItem(item))
            return true;

    return mKeys.GetInventory().HaveItem(item);
}

void Party::SetMoney(Royals royals)
{
    mGold = royals;
}

void Party::GainMoney(Royals royals)
{
    mGold.mValue += royals.mValue;
}

void Party::LoseMoney(Royals royals)
{
    if (mGold.mValue < royals.mValue)
    {
        mGold = Royals{0};
    }
    else
    {
        mGold.mValue -= royals.mValue;
    }
}

void Party::RemoveItem(unsigned itemIndex, unsigned quantity)
{
    if (ItemIndex{itemIndex} == sSovereigns)
    {
        LoseMoney(GetRoyals(Sovereigns{quantity}));
    }
    else if (ItemIndex{itemIndex} == sRoyals)
    {
        LoseMoney(Royals{quantity});
    }
    else
    {
        auto item = InventoryItemFactory::MakeItem(
            ItemIndex{itemIndex},
            static_cast<std::uint8_t>(quantity));
        if (item.IsKey())
            mKeys.RemoveItem(item);
        else
            for (const auto& character : mActiveCharacters)
                if (GetCharacter(character).RemoveItem(item)) return;
    }
}

void Party::AddItem(const InventoryItem& item)
{
    if (item.IsMoney())
        GainItem(item.GetItemIndex().mValue, item.GetQuantity());
    else if (item.IsKey())
        mKeys.GiveItem(item);
}

void Party::GainItem(unsigned itemIndex, unsigned quantity)
{
    if (ItemIndex{itemIndex} == sSovereigns)
    {
        mGold.mValue += GetRoyals(Sovereigns{quantity}).mValue;
        return;
    }
    else if (ItemIndex{itemIndex} == sRoyals)
    {
        mGold.mValue += quantity;
        return;
    }

    auto baseItem = InventoryItemFactory::MakeItem(
        ItemIndex{itemIndex},
        static_cast<std::uint8_t>(quantity));
    if (baseItem.IsKey())
    {
        mKeys.GiveItem(baseItem);
        return;
    }

    ForEachActiveCharacter([&](auto& character)
    {
        character.GiveItem(baseItem);
        return Loop::Continue;
    });
}

ActiveCharIndex Party::NextActiveCharacter(ActiveCharIndex currentCharacter) const
{
    unsigned i = currentCharacter.mValue;
    if (++i == mActiveCharacters.size())
        i = 0;
    return ActiveCharIndex{i};
}

std::pair<CharIndex, unsigned> Party::GetSkill(BAK::SkillType skill, bool best)
{
    std::optional<unsigned> skillValue{};
    auto character = CharIndex{0};
    for (unsigned i = 0; i < mActiveCharacters.size(); i++)
    {
        const auto charSkill = GetCharacter(ActiveCharIndex{i}).GetSkill(skill);
        Logging::LogDebug(__FUNCTION__) << "Char: " << GetCharacter(ActiveCharIndex{i}).mName << " skill: " << charSkill << "\n";
        if (!skillValue
            || best
                ? charSkill > skillValue
                : charSkill < skillValue)
        {
            skillValue = charSkill;
            character = mActiveCharacters[i];
        }
    }
    ASSERT(skillValue);
    return std::make_pair(character, *skillValue);
}

void Party::ImproveSkillForAll(SkillType skill, SkillChange skillChangeType, int multiplier)
{
    for (const auto c : mActiveCharacters)
    {
        GetCharacter(c).ImproveSkill(
            skill,
            skillChangeType,
            multiplier);
    }
}

std::ostream& operator<<(std::ostream& os, const Party& party)
{
    os << "Party {\n\tGold: " << party.mGold << " royals\n";
    os << "\tKeys: " << party.mKeys << "\n";
    os << "\tCharacters: \n";
    for (const auto& c : party.mCharacters)
        os << c;
    os << "\tActive: ";
    for (const auto i : party.mActiveCharacters)
        os << " " << party.mCharacters[i.mValue].mName << " ";
    os << "\n}";

    return os;
}

}
