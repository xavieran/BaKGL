#include "bak/spells.hpp"

#include "bak/fileBufferFactory.hpp"
#include "bak/monster.hpp"

#include "com/assert.hpp"
#include "com/bits.hpp"
#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "graphics/glm.hpp"

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

Spells::Spells(std::array<std::uint8_t, 6> spells)
{
    std::copy(spells.data(), spells.data() + 6, reinterpret_cast<std::uint8_t*>(&mSpellBytes));
    for (std::uint64_t i = 0; i < 8 * 6; i++)
    {
        if (HaveSpell(SpellIndex{i}))
        {
            mSpellIndices.emplace_back(SpellIndex{i});
        }
    }
}

bool Spells::HaveSpell(SpellIndex spellIndex) const
{
    return CheckBitSet(mSpellBytes, spellIndex.mValue);
}

void Spells::SetSpell(SpellIndex spellIndex)
{
    if (!HaveSpell(spellIndex))
    {
        mSpellIndices.emplace_back(spellIndex);
    }
    mSpellBytes = SetBit(mSpellBytes, spellIndex.mValue, true);
}

const std::uint64_t& Spells::GetSpellBytes() const
{
    return mSpellBytes;
}

std::vector<SpellIndex> Spells::GetSpells() const
{
    return mSpellIndices;
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

Symbol::Symbol(unsigned index)
{
    assert(index > 0 && index < 7);
    std::stringstream ss{};
    ss << "SYMBOL" << index << ".DAT";
    auto fb = FileBufferFactory::Get().CreateDataBuffer(ss.str());

    auto slotCount = fb.GetUint16LE();

    Logging::LogDebug(__FUNCTION__) << "Loading SymbolIndex #" << index << "\n";
    Logging::LogDebug(__FUNCTION__) << " slots: " << slotCount << "\n";
    for (unsigned i = 0; i < slotCount; i++)
    {
        auto spell = SpellIndex{fb.GetUint16LE()};
        auto position = fb.LoadVector<std::uint16_t, 2>();
        auto symbolIcon = fb.GetUint8();
        mSymbolSlots.emplace_back(SymbolSlot{spell, symbolIcon, position});
        Logging::LogDebug(__FUNCTION__) << " spell: " << spell << "  icon: " << +symbolIcon << " @ " << position << "\n";
    }
}

const std::vector<SymbolSlot>& Symbol::GetSymbolSlots() const
{
    return mSymbolSlots;
}

const SpellDatabase& SpellDatabase::Get()
{
    static SpellDatabase spellDb{};
    return spellDb;
}

const std::vector<Spell>& SpellDatabase::GetSpells() const
{
    return mSpells;
}

std::string_view SpellDatabase::GetSpellName(SpellIndex spellIndex) const
{
    ASSERT(spellIndex.mValue < mSpells.size());
    return mSpells[spellIndex.mValue].mName;
}

const Spell& SpellDatabase::GetSpell(SpellIndex spellIndex) const
{
    ASSERT(spellIndex.mValue < mSpells.size());
    return mSpells[spellIndex.mValue];
}

const SpellDoc& SpellDatabase::GetSpellDoc(SpellIndex spellIndex) const
{
    ASSERT(spellIndex.mValue < mSpells.size());
    return mSpellDocs[spellIndex.mValue];
}

const std::vector<Symbol>& SpellDatabase::GetSymbols() const
{
    return mSymbols;
}

SpellDatabase::SpellDatabase()
:
    mSpells{},
    mSpellDocs{}
{
    LoadSpells();
    LoadSpellDoc();
    //LoadSpellWeaknesses();
    //LoadSpellResistances();

    for (unsigned i = 1; i < 7; i++)
    {
        mSymbols.emplace_back(BAK::Symbol{i});
    }
}


void SpellDatabase::LoadSpells()
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer(sSpellNamesFile);
    const auto spells = fb.GetUint16LE();
    auto nameOffsets = std::vector<unsigned>{};
    for (unsigned i = 0; i < spells; i++)
    {
        unsigned nameOffset = fb.GetUint16LE();
        nameOffsets.emplace_back(nameOffset);
        unsigned minCost = fb.GetUint16LE();
        unsigned maxCost = fb.GetUint16LE();
        auto isMartialFlag = fb.GetUint16LE();
        assert(isMartialFlag == 0 || isMartialFlag == 1);
        bool isMartial =  isMartialFlag == 0x1; // name taken from SPELLREQ.DAT
        auto targetingType = fb.GetUint16LE();
        // targeting type - this seems to be affected by the effectAnimationType
        // e.g. when using effect animation 12 (winds of eortis), 0-3 only target enemies with LOS
        // 0 - only targets enemies - LOS
        // 1 - only target enemies - ignores LOS
        // 2 - targets allies - ignores LOS
        // 3 - targets allies - ignores LOS
        // 4 - targes enemies - ignores LOS
        // 5 - targets empty squares
        // 6 - targets empty squares
        // Color of any related effect sprites, e.g. sparks of flamecast, mind melt color
        auto color = fb.GetUint16LE();
        // Determines whether we throw a ball (flamecast), strike an enemy,
        // what kind of animation is used. Combines weirdly with the actual spell
        auto effectAnimationType = fb.GetUint16LE();
        // object required to cast spell
        auto objectRequired = ItemIndex{fb.GetUint16LE()};
        auto calculationType = static_cast<SpellCalculationType>(fb.GetUint16LE());
        int damage = fb.GetSint16LE();
        unsigned duration = fb.GetSint16LE();
        mSpells.emplace_back(Spell{
            SpellIndex{i},
            "",
            minCost,
            maxCost,
            isMartial,
            targetingType,
            (color != 0xffff) ? std::make_optional(color) : std::nullopt,
            (effectAnimationType != 0xffff) ? std::make_optional(effectAnimationType) : std::nullopt,
            (objectRequired.mValue != 0xffff) ? std::make_optional(objectRequired) : std::nullopt,
            calculationType,
            damage,
            duration});
    }
    fb.GetUint16LE();
    auto here = fb.Tell();
    for (unsigned i = 0; i < spells; i++)
    {
        fb.Seek(here + nameOffsets[i]);
        mSpells[i].mName = fb.GetString();
        Logging::LogDebug(__FUNCTION__) << mSpells[i] << "\n";
    }
}

void SpellDatabase::LoadSpellDoc()
{
    assert(!mSpells.empty());
    auto fb = FileBufferFactory::Get().CreateDataBuffer(sSpellDocsFile);
    const auto offsetCount = fb.GetUint16LE();
    auto stringOffsets = std::vector<unsigned>{};
    for (unsigned i = 0; i < offsetCount; i++)
    {
        stringOffsets.emplace_back(fb.GetUint32LE());
    }

    fb.Skip(2);

    auto here = fb.Tell();
    for (unsigned i = 0, entry = 0; i < mSpells.size(); i++)
    {
        fb.Seek(here + stringOffsets[entry++]);
        auto title = fb.GetString();
        fb.Seek(here + stringOffsets[entry++]);
        auto cost = fb.GetString();
        fb.Seek(here + stringOffsets[entry++]);
        auto damage = fb.GetString();
        fb.Seek(here + stringOffsets[entry++]);
        auto duration = fb.GetString();
        fb.Seek(here + stringOffsets[entry++]);
        auto lineOfSight = fb.GetString();
        fb.Seek(here + stringOffsets[entry++]);
        auto description = fb.GetString();
        fb.Seek(here + stringOffsets[entry++]);
        description += " " + fb.GetString();
        mSpellDocs.emplace_back(
            SpellDoc{
                SpellIndex{i},
                title,
                cost,
                damage,
                duration,
                lineOfSight,
                description});
    }

    for (unsigned i = 0; i < mSpells.size(); i++)
    {
        auto doc = mSpellDocs[i];
        Logging::LogDebug(__FUNCTION__) << GetSpellName(SpellIndex{i}) << "\nTitle: " << doc.mTitle
            << "\nCost: " << doc.mCost << "\nDamage: " << doc.mDamage 
            << "\nDuration: " << doc.mDuration << "\nLOS: " << doc.mLineOfSight
            << "\nDescription: " << doc.mDescription << "\n";
    }
}

void SpellDatabase::LoadSpellWeaknesses()
{
    auto monsters = MonsterNames::Get();
    auto fb = FileBufferFactory::Get().CreateDataBuffer(sSpellWeaknessesFile);
    unsigned entries = fb.GetUint16LE();
    for (unsigned i = 0; i < entries; i++)
    {
        fb.Dump(6);
        auto spells = Spells{fb.GetArray<6>()};
        std::stringstream ss{};
        for (const auto& spell : mSpells)
        {
            if (spell.HasSpell(spells))
            {
                ss << spell.mName << ",";
            }
        }
        Logging::LogDebug(__FUNCTION__) << "Monster: " << i - 1 << std::dec << "(" << i - 1<< ") - "
            << monsters.GetMonsterName(MonsterIndex{i - 1}) << " (" << std::hex << spells.GetSpells() << std::dec << ") " << ss.str() << "\n";
    }
}

void SpellDatabase::LoadSpellResistances()
{
    auto monsters = MonsterNames::Get();
    auto fb = FileBufferFactory::Get().CreateDataBuffer(sSpellResistances);
    unsigned entries = fb.GetUint16LE();
    for (unsigned i = 0; i < entries; i++)
    {
        fb.Dump(6);
        auto spells = Spells{fb.GetArray<6>()};
        std::stringstream ss{};
        for (const auto& spell : mSpells)
        {
            if (spell.HasSpell(spells))
            {
                ss << spell.mName << ",";
            }
        }
        Logging::LogDebug(__FUNCTION__) << "Monster: " << i - 1 << std::dec << "(" << i - 1<< ") - "
            << monsters.GetMonsterName(MonsterIndex{i - 1}) << " " << ss.str() << "\n";
    }
}

const PowerRing& PowerRing::Get()
{
    static auto ring = PowerRing{};
    return ring;
}

const std::vector<glm::ivec2>& PowerRing::GetPoints() const
{
    return mPoints;
}

PowerRing::PowerRing()
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer("RING.DAT");
    unsigned points = 30;
    std::vector<int> xs{};
    std::vector<int> ys{};
    for (unsigned i = 0; i < points; i++)
    {
        xs.emplace_back(fb.GetSint16LE());
    }
    for (unsigned i = 0; i < points; i++)
    {
        ys.emplace_back(fb.GetSint16LE());
    }
    for (unsigned i = 0; i < points; i++)
    {
        mPoints.emplace_back(xs[i], ys[i]);
    }
}

std::vector<glm::vec2> SymbolLines::GetPoints(unsigned symbol)
{
    const auto offset = 0x1726;
    const unsigned xPointer = (sXPointers[symbol] - offset) >> 1;
    const unsigned yPointer = (sYPointers[symbol] - offset) >> 1;
    std::vector<glm::vec2> points{};
    for (unsigned i = 0; i < 6; i++)
    {
        auto x = sCoords[xPointer + i];
        auto y = sCoords[yPointer + i];
        points.emplace_back(glm::vec2{x, y});
    }
    return points;
}

SpellState::SpellState(std::uint16_t spells)
:
    mSpells{spells}
{}

bool SpellState::SpellActive(StaticSpells spell) const
{
    return CheckBitSet(mSpells, spell);
}

void SpellState::SetSpellState(StaticSpells spell, bool state)
{
    mSpells = SetBit(mSpells, spell, state);
}

std::uint16_t SpellState::GetSpells() const
{
    return mSpells;
}

}
