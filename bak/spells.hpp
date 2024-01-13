#pragma once

#include "com/assert.hpp"
#include "com/logger.hpp"
#include "com/ostream.hpp"

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

class Spell
{
public:
    unsigned mIndex;
    std::string mName;
    unsigned mMinCost;
    unsigned mMaxCost;
    std::array<std::uint8_t, 8> mUnknown;
    std::optional<ItemIndex> mObjectRequired; // Which COMBAT.TBL object is used for effect?
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
    static constexpr auto sSpellResistances = "SPELLRES.DAT";
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

        fb.GetUint16LE();

        Logging::LogDebug(__FUNCTION__) << "String: " << stringOffsets << " currentLoc: " << fb.Tell() << "\n";
        auto here = fb.Tell();
        for (unsigned i = 0, entry = 0; i < mSpells.size(); i++)
        {
            Logging::LogDebug(__FUNCTION__) << "Entry: " << entry << "\n";
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
            auto u0 = fb.GetUint16LE();
            auto u1 = fb.GetUint16LE();
            auto u2 = fb.GetUint16LE();
            Logging::LogDebug(__FUNCTION__) << "Monster: " << i - 1 << std::dec << "(" << i - 1<< ") - " << monsters.GetMonsterName(MonsterIndex{i - 1}) << std::hex << " datas: " << u0 << " " << u1 << " " << u2 << "\n";
            for (unsigned j = 0; j < 16; j++)
            {
                if ((u0 & (1 << j)) != 0)
                {
                    Logging::LogDebug(__FUNCTION__) << "  " << mSpells[j].mName << "\n";
                }
            }
            for (unsigned j = 0; j < 16; j++)
            {
                if ((u1 & (1 << j)) != 0)
                {
                    Logging::LogDebug(__FUNCTION__) << "  " << mSpells[16 + j].mName << "\n";
                }
            }
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
            auto u0 = fb.GetUint16LE();
            auto u1 = fb.GetUint16LE();
            auto u2 = fb.GetUint16LE();
            Logging::LogDebug(__FUNCTION__) << "Monster: " << i - 1 << std::dec << "(" << i - 1<< ") - " << monsters.GetMonsterName(MonsterIndex{i - 1}) << std::hex << " datas: " << u0 << " " << u1 << " " << u2 << "\n";
            for (unsigned j = 0; j < 16; j++)
            {
                if ((u0 & (1 << j)) != 0)
                {
                    Logging::LogDebug(__FUNCTION__) << "  " << mSpells[j].mName << "\n";
                }
            }
            for (unsigned j = 0; j < 16; j++)
            {
                if ((u1 & (1 << j)) != 0)
                {
                    Logging::LogDebug(__FUNCTION__) << "  " << mSpells[16 + j].mName << "\n";
                }
            }
            for (unsigned j = 0; j < 16; j++)
            {
                if ((u1 & (1 << j)) != 0)
                {
                    if (32 + j < mSpells.size())
                    {
                        Logging::LogDebug(__FUNCTION__) << "  " << mSpells[32 + j].mName << "\n";
                    }
                }
            }
        }
    }

private:
    std::vector<Spell> mSpells;
    std::vector<SpellDoc> mSpellDocs;
};

}
