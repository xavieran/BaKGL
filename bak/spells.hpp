#pragma once

#include "com/assert.hpp"
#include "com/bits.hpp"
#include "com/logger.hpp"
#include "com/ostream.hpp"
#include "graphics/glm.hpp"

#include "bak/fileBufferFactory.hpp"
#include "bak/types.hpp"
#include "bak/monster.hpp"

#include <string_view>
#include <vector>

namespace BAK {

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

class Spells
{
public:
    explicit Spells(std::array<std::uint8_t, 6> spells)
    {
        std::copy(spells.data(), spells.data() + 6, reinterpret_cast<std::uint8_t*>(&mSpells));
    }

    bool HaveSpell(std::uint64_t spellIndex) const
    {
        return CheckBitSet(mSpells, spellIndex);
    }

    void SetSpell(std::uint64_t spellIndex)
    {
        mSpells = SetBit(mSpells, spellIndex, true);
    }

    std::uint64_t mSpells;
};

std::ostream& operator<<(std::ostream&, const Spells&);

class Spell
{
public:
    bool HasSpell(Spells spells) const
    {
        return (spells.mSpells & (static_cast<std::uint64_t>(1) << mIndex)) != 0;
    }

    unsigned mIndex;
    std::string mName;
    unsigned mMinCost;
    unsigned mMaxCost;
    std::array<std::uint8_t, 8> mUnknown;
    std::optional<ItemIndex> mObjectRequired;
    SpellCalculationType mCalculationType;
    int mDamage;
    unsigned mDuration;
};

std::ostream& operator<<(std::ostream&, const Spell&);

class SpellDoc
{
public:
    unsigned mIndex;
    std::string mTitle;
    std::string mCost;
    std::string mDamage;
    std::string mDuration;
    std::string mLineOfSight;
    std::string mDescription;
};

class SpellInfo
{
    static constexpr auto sSpellNamesFile = "SPELLS.DAT";
    static constexpr auto sSpellDocsFile  = "SPELLDOC.DAT";
    static constexpr auto sSpellWeaknessesFile = "SPELLWEA.DAT";
    static constexpr auto sSpellResistances    = "SPELLRES.DAT";

    static constexpr auto sSymbol1 = "SYMBOL1.DAT";
    static constexpr auto sSymbol2 = "SYMBOL2.DAT";
    static constexpr auto sSymbol3 = "SYMBOL3.DAT";
    static constexpr auto sSymbol4 = "SYMBOL4.DAT";
    static constexpr auto sSymbol5 = "SYMBOL5.DAT";
    static constexpr auto sSymbol6 = "SYMBOL6.DAT";
public:
    SpellInfo()
    :
        mSpells{},
        mSpellDocs{}
    {
        LoadSpells();
        LoadSpellDoc();
        LoadSpellWeaknesses();
        LoadSpellResistances();
    }

    const auto& GetSpells() const
    {
        return mSpells;
    }

    std::string_view GetSpellName(unsigned spellIndex) const
    {
        ASSERT(spellIndex < mSpells.size());
        return mSpells[spellIndex].mName;
    }

    const SpellDoc& GetSpellDoc(unsigned spellIndex) const
    {
        ASSERT(spellIndex < mSpells.size());
        return mSpellDocs[spellIndex];
    }

private:
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
            auto unknown = fb.GetArray<8>();
            auto objectRequired = ItemIndex{fb.GetUint16LE()};
            auto calculationType = static_cast<SpellCalculationType>(fb.GetUint16LE());
            int damage = fb.GetSint16LE();
            unsigned duration = fb.GetSint16LE();
            mSpells.emplace_back(Spell{
                i,
                "",
                minCost, maxCost,
                unknown,
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
                    i,
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
            Logging::LogDebug(__FUNCTION__) << GetSpellName(i) << "\nTitle: " << doc.mTitle
                << "\nCost: " << doc.mCost << "\nDamage: " << doc.mDamage 
                << "\nDuration: " << doc.mDuration << "\nLOS: " << doc.mLineOfSight
                << "\nDescription: " << doc.mDescription << "\n";
        }
    }

    void LoadSpellWeaknesses()
    {
        auto monsters = MonsterNames{};
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
                << monsters.GetMonsterName(MonsterIndex{i - 1}) << " (" << std::hex << spells.mSpells << std::dec << ") " << ss.str() << "\n";
        }
    }

    void LoadSpellResistances()
    {
        auto monsters = MonsterNames{};
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
};

class SymbolCoordinates
{
public:

    struct SymbolSlot
    {
        unsigned mSymbolIcon;
        glm::vec<2, std::uint16_t> mPosition;
    };

    explicit SymbolCoordinates(unsigned index)
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
            auto unknown = fb.GetUint16LE();
            auto position = fb.LoadVector<std::uint16_t, 2>();
            auto symbolIcon = fb.GetUint8();
            mSymbolSlots.emplace_back(SymbolSlot{symbolIcon, position});
            Logging::LogDebug(__FUNCTION__) << " unk: " << unknown << "  icon: " << +symbolIcon << " @ " << position << "\n";
        }
    }

    const auto& GetSymbolSlots() const
    {
        return mSymbolSlots;
    }

private:
    std::vector<SymbolSlot> mSymbolSlots;
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

class CastScreen
{
public:
};

}
