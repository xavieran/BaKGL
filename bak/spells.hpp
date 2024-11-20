#pragma once

#include "bak/types.hpp"

#include <glm/glm.hpp>

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

std::ostream& operator<<(std::ostream&, StaticSpells);

StaticSpells ToStaticSpell(SpellIndex);

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
    Spells(std::array<std::uint8_t, 6> spells);

    bool HaveSpell(SpellIndex spellIndex) const;
    void SetSpell(SpellIndex spellIndex);
    const std::uint64_t& GetSpellBytes() const;
    std::vector<SpellIndex> GetSpells() const;

private:
    std::uint64_t mSpellBytes{};
    std::vector<SpellIndex> mSpellIndices{};
};

std::ostream& operator<<(std::ostream&, const Spells&);

class Spell
{
public:
    bool HasSpell(Spells spells) const
    {
        return spells.HaveSpell(mIndex);
    }

    SpellIndex mIndex{};
    std::string mName{};
    unsigned mMinCost{};
    unsigned mMaxCost{};
    bool mIsMartial{};
    std::uint16_t mTargetingType{};
    std::optional<std::uint16_t> mColor{};
    std::optional<std::uint16_t> mAnimationEffectType{};
    std::optional<ItemIndex> mObjectRequired{};
    SpellCalculationType mCalculationType{};
    int mDamage{};
    unsigned mDuration{};
};

std::ostream& operator<<(std::ostream&, const Spell&);

class SpellDoc
{
public:
    SpellIndex mIndex{};
    std::string mTitle{};
    std::string mCost{};
    std::string mDamage{};
    std::string mDuration{};
    std::string mLineOfSight{};
    std::string mDescription{};
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

    explicit Symbol(unsigned index);
    const std::vector<SymbolSlot>& GetSymbolSlots() const;

private:
    std::vector<SymbolSlot> mSymbolSlots{};
};

class SpellDatabase
{
    static constexpr auto sSpellNamesFile = "SPELLS.DAT";
    static constexpr auto sSpellDocsFile  = "SPELLDOC.DAT";
    static constexpr auto sSpellWeaknessesFile = "SPELLWEA.DAT";
    static constexpr auto sSpellResistances    = "SPELLRES.DAT";

public:
    static const SpellDatabase& Get();

    const std::vector<Spell>& GetSpells() const;
    std::string_view GetSpellName(SpellIndex spellIndex) const;
    const Spell& GetSpell(SpellIndex spellIndex) const;
    const SpellDoc& GetSpellDoc(SpellIndex spellIndex) const;
    const std::vector<Symbol>& GetSymbols() const;

private:
    SpellDatabase();

    void LoadSpells();
    void LoadSpellDoc();
    void LoadSpellWeaknesses();
    void LoadSpellResistances();

private:
    std::vector<Spell> mSpells{};
    std::vector<SpellDoc> mSpellDocs{};
    std::vector<Symbol> mSymbols{};
};

// These are the locations of the dots for the spell power ring
class PowerRing
{
public:
    static const PowerRing& Get();

    const std::vector<glm::ivec2>& GetPoints() const;
private:
    PowerRing();

    std::vector<glm::ivec2> mPoints{};
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
    static constexpr std::uint16_t sXPointers [] = {
        0x1726, 0x179E, 0x173E, 0x1756, 0x176E, 0x1786, 0x17B6
    };
    static constexpr std::uint16_t sYPointers [] = {
        0x1732, 0x17AA, 0x174A, 0x1762, 0x177A, 0x1792, 0x17C2
    };

public:
    static std::vector<glm::vec2> GetPoints(unsigned symbol);
};

class SpellState
{
public:
    SpellState() = default;
    explicit SpellState(std::uint16_t spells);

    bool SpellActive(StaticSpells spell) const;
    void SetSpellState(StaticSpells spell, bool state);
    std::uint16_t GetSpells() const;
private:
    std::uint16_t mSpells{};
};

}
