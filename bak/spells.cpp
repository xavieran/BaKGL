#include "bak/spells.hpp"

#include "bak/objectInfo.hpp"

#include <iostream>

namespace BAK {

std::ostream& operator<<(std::ostream& os, StaticSpells s)
{
    using enum StaticSpells;
    switch (s)
    {
    case DragonsBreath: return os << "Dragons Breath";
    case CandleGlow: return os << "Candle Glow";
    case Stardusk: return os << "Stardusk";
    case AndTheLightShallLie: return os << " And The Light Shall Lie";
    case Union: return os << "Union";
    case ScentOfSarig: return os << "Scent Of Sarig";
    default: return os << "Unknown Static Spell [" << static_cast<unsigned>(s) << "]";
    }
}

StaticSpells ToStaticSpell(SpellIndex s)
{
    switch (s.mValue)
    {
        case 0: return StaticSpells::DragonsBreath;
        case 2: return StaticSpells::CandleGlow;
        case 26: return StaticSpells::Stardusk;
        case 34: return StaticSpells::Union;
        case 35: return StaticSpells::AndTheLightShallLie;
        case 8: return StaticSpells::ScentOfSarig;
        default: return static_cast<StaticSpells>(-1);
    }
}

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
    return os << "Spells{" << std::hex << s.GetSpells() << std::dec << "}";
}

std::ostream& operator<<(std::ostream& os, const Spell& s)
{
    return os << "Spell{" << s.mIndex << " - " << s.mName << " minCost: " << s.mMinCost 
        << " maxCost: " << s.mMaxCost <<  " isCmbt: " << s.mIsMartial 
        << " targTp: " << s.mTargetingType
        << " effCol: " << s.mColor << " anim: " << s.mAnimationEffectType 
        << " objReq: " << s.mObjectRequired << " (" <<
        (s.mObjectRequired ? *s.mObjectRequired : ItemIndex{0}) << ")"
        << " calcTp: " << s.mCalculationType << " dmg: " << s.mDamage
        << " len: " << s.mDuration << "}";
}

}
