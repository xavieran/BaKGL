#include "bak/haggle.hpp"

#include "bak/money.hpp"
#include "bak/party.hpp"
#include "bak/shop.hpp"
#include "bak/skills.hpp"

#include "com/random.hpp"

#include <algorithm>

namespace BAK::Haggle {

int HaggleRollRandomNumber(int skillValue)
{
    int maxRoll = 0;
    for (unsigned i = 0; i < 3; i++)
    {
        const auto roll = static_cast<int>(GetRandomNumber(0, 0xfff) % skillValue);
        if (roll > maxRoll)
        {
            maxRoll = roll;
        }
    }
    return maxRoll;
}

std::optional<unsigned> DoFailHaggle(Party& party, const ShopStats& shop, int randomSkillFactor)
{
    const int skillImprovedTest = GetRandomNumber(0, 0xfff) % 100;
    // the higher the skill, the less likely we train it
    const auto skillTrainThreshold = (100 - randomSkillFactor) / 5;
    if (skillImprovedTest < skillTrainThreshold)
    {
        party.ImproveSkillForAll(SkillType::Haggling, SkillChange::ExercisedSkill, 1);
    }

    const auto shopAnnoyedTest = GetRandomNumber(0, 0xfff) % 100;
    if (shopAnnoyedTest < shop.mHaggleAnnoyanceFactor)
    {
        return std::make_optional(sUnpurchaseablePrice.mValue);
    }

    return std::nullopt;
}

std::pair<unsigned, unsigned> GetHaggleWinMargin(unsigned characterSkillValue, unsigned shopHaggleDifficulty)
{
    const auto randomSkillFactor = HaggleRollRandomNumber(characterSkillValue);
    const auto randomShopHaggleFactor = HaggleRollRandomNumber(shopHaggleDifficulty);
    return std::make_pair(
        randomSkillFactor,
        std::clamp(randomSkillFactor - randomShopHaggleFactor, 0, randomSkillFactor));
}

std::optional<unsigned> TryHaggle(
    Party& party,
    ActiveCharIndex character,
    ShopStats& shop,
    ItemIndex item,
    int shopCurrentDiscount)
{
    const auto itemValue = ObjectIndex::Get().GetObject(item).mValue;
    const auto scaledShopValue = (shop.mSellFactor + 100) * itemValue;
    const int shopBasicValue = scaledShopValue / 100;
    const auto shopBasicValueRemainder = scaledShopValue % 100;

    if (shop.mMaxDiscount == 0)
    {
        return std::nullopt;
    }

    // Have already haggled successfully if this is the case
    if (shopCurrentDiscount != 0)
    {
        return DoFailHaggle(party, shop, 0x1);
    }

    const auto characterSkillValue = party.GetCharacter(character).GetSkill(SkillType::Haggling);
    const auto [randomSkillFactor, haggleWinMargin] = GetHaggleWinMargin(characterSkillValue, shop.mHaggleDifficulty);

    if (haggleWinMargin > 0)
    {
        const int discount = ((shop.mMaxDiscount - shopBasicValueRemainder) >> 1)
            + haggleWinMargin;

        const auto randomDiscount = std::clamp(
            HaggleRollRandomNumber(discount),
            0, static_cast<int>(shop.mMaxDiscount));

        const int discountAmount = (shopBasicValue * randomDiscount) / 100;

        party.ImproveSkillForAll(SkillType::Haggling, SkillChange::ExercisedSkill, 1);
        party.GetCharacter(character).ImproveSkill(SkillType::Haggling, SkillChange::ExercisedSkill, 1);

        return std::make_optional(discountAmount);
    }
    else
    {
        return DoFailHaggle(party, shop, randomSkillFactor);
    }
}

}
