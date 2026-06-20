#pragma once

#include "bak/character.hpp"
#include "bak/keyContainer.hpp"
#include "bak/types.hpp"


#include <vector>

namespace BAK {

class Party
{
public:
    Party() = delete;
    Party(const Party&) = delete;
    Party& operator=(const Party&) = delete;

    Party(Party&&) = default;
    Party& operator=(Party&&) = default;

    Party(
        Royals,
        KeyContainer&&,
        std::vector<Inventory>&&,
        std::vector<Character>&&,
        std::vector<CharIndex>&&);

    std::size_t GetNumCharacters() const;
    const Character& GetCharacter(CharIndex i) const;
    Character& GetCharacter(CharIndex i);

    std::optional<ActiveCharIndex> FindActiveCharacter(CharIndex index) const;
    std::optional<ActiveCharIndex> GetSpellcaster() const;

    const Character& GetCharacter(ActiveCharIndex i) const;
    Character& GetCharacter(ActiveCharIndex i);

    void SetActiveCharacters(const std::vector<CharIndex>& characters);

    const KeyContainer& GetKeys() const;
    KeyContainer& GetKeys();

    Royals GetGold() const;

    bool HaveItem(ItemIndex itemIndex) const;

    void SetMoney(Royals royals);
    void GainMoney(Royals royals);
    void LoseMoney(Royals royals);

    void RemoveItem(unsigned itemIndex, unsigned quantity);
    void AddItem(const InventoryItem& item);
    void GainItem(unsigned itemIndex, unsigned quantity);

    ActiveCharIndex NextActiveCharacter(ActiveCharIndex currentCharacter) const;

    std::pair<CharIndex, unsigned> GetSkill(BAK::SkillType skill, bool best);
    void ImproveSkillForAll(SkillType skill, SkillChange skillChangeType, int multiplier);

    template <typename F>
    void ForEachActiveCharacter(F&& f) const
    {
        for (const auto character : mActiveCharacters)
        {
            if (std::forward<F>(f)(GetCharacter(character)) == Loop::Finish)
            {
                return;
            }
        }
    }

    template <typename F>
    void ForEachActiveCharacter(F&& f)
    {
        for (const auto character : mActiveCharacters)
        {
            if (std::forward<F>(f)(GetCharacter(character)) == Loop::Finish)
            {
                return;
            }
        }
    }

    Royals mGold;
    KeyContainer mKeys;

    std::vector<Inventory> mInventories;
    std::vector<Character> mCharacters;
    std::vector<CharIndex> mActiveCharacters;
};

std::ostream& operator<<(std::ostream&, const Party&);

}
