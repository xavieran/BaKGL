#pragma once

#include "bak/fileBufferFactory.hpp"
#include "bak/types.hpp"
#include "bak/monster.hpp"

#include "com/assert.hpp"
#include "com/bits.hpp"
#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "graphics/glm.hpp"

#include <string_view>
#include <vector>

namespace BAK {

enum class StaticSpells : std::uint16_t
{
    DragonsBreath = 0,
    CandleGlow = 1,
    Stardusk = 2,
    AndTheLightShallLie = 3,
    Union = 4,
    ScentOfSarig = 5
};

static constexpr std::array<std::uint8_t, 6> sStaticSpellMapping = {
    30, 31, 32, 34, 33, 35
};

enum class SpellCalculationType
{
    NonCostRelated,
    FixedAmount,
    CostTimesDamage,
    CostTimesDuration,
    Special1,
    Special2
};

std::string_view ToString(SpellCalculationType);
std::ostream& operator<<(std::ostream&, SpellCalculationType);

static constexpr auto DragonsBreath = SpellIndex{0};
// actually this appears as index 8 in the SPELLS.DAT table... ???
static constexpr auto ScentOfSarig = SpellIndex{5};

class Spells
{
public:
    explicit Spells(std::array<std::uint8_t, 6> spells)
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

    bool HaveSpell(SpellIndex spellIndex) const
    {
        return CheckBitSet(mSpellBytes, spellIndex.mValue);
    }

    void SetSpell(SpellIndex spellIndex)
    {
        if (!HaveSpell(spellIndex))
        {
            mSpellIndices.emplace_back(spellIndex);
        }
        mSpellBytes = SetBit(mSpellBytes, spellIndex.mValue, true);
    }

    const std::uint64_t& GetSpellBytes() const
    {
        return mSpellBytes;
    }

    auto GetSpells() const
    {
        return mSpellIndices;
    }

private:
    std::uint64_t mSpellBytes;
    std::vector<SpellIndex> mSpellIndices;
};

std::ostream& operator<<(std::ostream&, const Spells&);

class Spell
{
public:
    bool HasSpell(Spells spells) const
    {
        return spells.HaveSpell(mIndex);
    }

    SpellIndex mIndex;
    std::string mName;
    unsigned mMinCost;
    unsigned mMaxCost;
    bool mIsMartial;
    std::uint16_t mTargetingType;
    std::optional<std::uint16_t> mColor;
    std::optional<std::uint16_t> mAnimationEffectType;
    std::optional<ItemIndex> mObjectRequired;
    SpellCalculationType mCalculationType;
    int mDamage;
    unsigned mDuration;
};

std::ostream& operator<<(std::ostream&, const Spell&);

class SpellDoc
{
public:
    SpellIndex mIndex;
    std::string mTitle;
    std::string mCost;
    std::string mDamage;
    std::string mDuration;
    std::string mLineOfSight;
    std::string mDescription;
};

class Symbol
{
public:

    struct SymbolSlot
    {
        SpellIndex mSpell;
        unsigned mSpellIcon;
        glm::vec<2, std::uint16_t> mPosition;
    };

    explicit Symbol(unsigned index)
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

    const auto& GetSymbolSlots() const
    {
        return mSymbolSlots;
    }

private:
    std::vector<SymbolSlot> mSymbolSlots;
};

class SpellDatabase
{
    static constexpr auto sSpellNamesFile = "SPELLS.DAT";
    static constexpr auto sSpellDocsFile  = "SPELLDOC.DAT";
    static constexpr auto sSpellWeaknessesFile = "SPELLWEA.DAT";
    static constexpr auto sSpellResistances    = "SPELLRES.DAT";

public:
    static const SpellDatabase& Get()
    {
        static SpellDatabase spellDb{};
        return spellDb;
    }

    const auto& GetSpells() const
    {
        return mSpells;
    }

    std::string_view GetSpellName(SpellIndex spellIndex) const
    {
        ASSERT(spellIndex.mValue < mSpells.size());
        return mSpells[spellIndex.mValue].mName;
    }

    const SpellDoc& GetSpellDoc(SpellIndex spellIndex) const
    {
        ASSERT(spellIndex.mValue < mSpells.size());
        return mSpellDocs[spellIndex.mValue];
    }

    const auto& GetSymbols() const
    {
        return mSymbols;
    }

private:
    SpellDatabase()
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


    void LoadSpells()
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

    void LoadSpellDoc()
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

    void LoadSpellWeaknesses()
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

    void LoadSpellResistances()
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

private:
    std::vector<Spell> mSpells;
    std::vector<SpellDoc> mSpellDocs;
    std::vector<Symbol> mSymbols;
};

// These are the locations of the dots for the spell power ring
class PowerRing
{
public:
    PowerRing()
    {
        auto fb = FileBufferFactory::Get().CreateDataBuffer("RING.DAT");
        unsigned points = 30;
        Logging::LogDebug(__FUNCTION__) << "Ring has: " << points << " points\n";
        for (unsigned i = 0; i < points; i++)
        {
            auto pos = fb.LoadVector<std::uint16_t, 2>();
            Logging::LogDebug(__FUNCTION__) << "  " << i << " - " << pos << "\n";
            mPoints.emplace_back(pos);
        }
    }

    const auto& GetPoints() const
    {
        return mPoints;
    }
private:
    std::vector<glm::vec<2, std::uint16_t>> mPoints;
};

class SymbolLines
{
    static constexpr std::uint16_t sCoords [] = {
        0x47, 0x5F, 0x7B, 0x13, 0x31, 0x3B, 0x10, 0x65, 0x3D, 0x3D, 0x65,
        0x3D, 0x7C, 0x47, 0x28, 0x68, 0x47, 0x13, 0x3E, 0x10, 0x61, 0x61,
        0x10, 0x3E, 0x68, 0x13, 0x69, 0x27, 0x7B, 0x29, 0x62, 0x3E, 0x1A,
        0x1A, 0x3D, 0x62, 0x25, 0x25, 0x6B, 0x6B, 0x25, 0x6B, 0x1D, 0x60,
        0x1D, 0x60, 0x60, 0x60, 0x4B, 0x72, 0x4B, 0x4B, 0x1F, 0x4B, 0x48,
        0x59, 0x11, 0x48, 0x59, 0x11, 0x13, 0x2F, 0x6C, 0x22, 0x5C, 0x7B,
        0x46, 0x16, 0x5D, 0x5D, 0x14, 0x46, 0x46, 0x73, 0x75, 0x48, 0x1A,
        0x19, 0x10, 0x25, 0x53, 0x6B, 0x56, 0x28
    };
    static constexpr std::uint16_t sStartPointers [] = {
        0x1726, 0x179E, 0x173E, 0x1756, 0x176E, 0x1786, 0x17B6
    };
    static constexpr std::uint16_t sEndPointers [] = {
        0x1732, 0x17AA, 0x174A, 0x1762, 0x177A, 0x1792, 0x17C2
    };

public:
    static std::vector<glm::vec2> GetPoints(unsigned symbol)
    {
        const auto offset = 0x1726;
        const unsigned startPointer = (sStartPointers[symbol] - offset) >> 1;
        const unsigned endPointer = (sEndPointers[symbol] - offset) >> 1;
        std::vector<glm::vec2> points{};
        for (unsigned i = 0; i < 6; i++)
        {
            auto x = sCoords[startPointer + i];
            auto y = sCoords[endPointer + i];
            points.emplace_back(glm::vec2{x, y});
        }
        return points;
    }
};

class SpellState
{
public:
    SpellState() = default;
    explicit SpellState(std::uint16_t spells)
    :
        mSpells{spells}
    {}

    bool SpellActive(StaticSpells spell) const
    {
        return CheckBitSet(mSpells, spell);
    }

    void SetSpellState(StaticSpells spell, bool state)
    {
        SetBit(mSpells, spell, state);
    }

    std::uint16_t GetSpells() const
    {
        return mSpells;
    }
private:
    std::uint16_t mSpells;
};

}
