#include "bak/spells.hpp"

#include "bak/objectInfo.hpp"

#include <iostream>

namespace BAK {

std::string_view ToString(SpellCalculationType s)
{
    using enum SpellCalculationType;
    switch (s)
    {
        case NonCostRelated: return "NonCostRelated";
        case FixedAmount: return "FixedAmount";
        case CostTimesDamage: return "CostTimesDamage";
        case CostTimesDuration: return "CostTimesDuration";
        case Special1: return "Special1";
        case Special2: return "Special2";
        default: return "Unknown";
    }
}

std::ostream& operator<<(std::ostream& os, SpellCalculationType s)
{
    return os << ToString(s);
}

std::ostream& operator<<(std::ostream& os, const Spells& s)
{
    return os << "Spells{" << std::hex << s.mSpells << std::dec << "}";
}

std::ostream& operator<<(std::ostream& os, const Spell& s)
{
    return os << "Spell{" << s.mIndex << " - " << s.mName << " minCost: " << s.mMinCost 
        << " maxCost: " << s.mMaxCost <<  " isCmbt: " << s.mIsMartial 
        << " targTp: " << s.mTargetingType
        << " effCol: " << s.mColor << " anim: " << s.mAnimationEffectType 
        << " objReq: " << s.mObjectRequired << " (" <<
        (s.mObjectRequired ? GetObjectIndex().GetObject(*s.mObjectRequired).mName : "None") << ")"
        << " calcTp: " << s.mCalculationType << " dmg: " << s.mDamage
        << " len: " << s.mDuration << "}";
}

}
