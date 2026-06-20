#include "bak/save/character.hpp"

#include "bak/save/containers.hpp"
#include "bak/save/saveOffsets.hpp"
#include "bak/fileBufferFactory.hpp"
#include "bak/state/skill.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"

namespace BAK {

std::vector<Inventory> LoadCharacterInventories(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger("LoadInventories");

    std::vector<Inventory> inventories;

    for (unsigned character = 0; character < SaveOffsets::sCharacterCount; character++)
    {
        inventories.emplace_back(LoadCharacterInventory(
            fb,
            SaveOffsets::GetCharacterInventoryOffset(character)));
    }

    return inventories;
}

std::vector<Character> LoadCharacters(FileBuffer& fb, std::vector<Inventory>& inventories)
{
    const auto& logger = Logging::LogState::GetLogger("LoadCharacters");
    unsigned characters = SaveOffsets::sCharacterCount;

    std::vector<Character> chars;

    for (unsigned character = 0; character < characters; character++)
    {
        fb.Seek(SaveOffsets::GetCharacterNameOffset(character));
        auto name = fb.GetString(SaveOffsets::sCharacterNameLength);

        fb.Seek(SaveOffsets::GetCharacterSkillOffset(character));
        logger.Spam() << "Name: " << name << "@"
            << std::hex << fb.Tell() << std::dec << "\n";

        auto characterNameOffset = fb.GetArray<2>();
        auto spells = Spells{fb.GetArray<6>()};

        auto skills = LoadSkills(fb);

        const auto pos = fb.Tell();
        for (unsigned i = 0; i < Skills::sSkills; i++)
        {
            const auto selected = State::ReadSkillSelected(fb, character, i);
            const auto unseenImprovement = State::ReadSkillUnseenImprovement(fb, character, i);

            skills.GetSkill(static_cast<SkillType>(i)).mSelected = selected;
            skills.GetSkill(static_cast<SkillType>(i)).mUnseenImprovement = unseenImprovement;
        }
        fb.Seek(pos);

        skills.SetSelectedSkillPool(skills.CalculateSelectedSkillPool());

        auto combatCharIndex = fb.GetUint8();
        auto unknown2 = fb.GetArray<6>();
        logger.Spam() << " Finished loading : " << name << std::hex << fb.Tell() << std::dec << "\n";

        auto conditions = LoadConditions(fb, character);

        chars.emplace_back(
            character,
            name,
            skills,
            spells,
            characterNameOffset,
            combatCharIndex,
            unknown2,
            conditions,
            &inventories[character]);

        auto affectors = GetCharacterSkillAffectors(fb, CharIndex{character});
        for (const auto& affector : affectors)
        {
            chars.back().AddSkillAffector(affector);
        }
    }

    return chars;
}

Inventory LoadCharacterInventory(FileBuffer& fb, unsigned offset)
{
    const auto& logger = Logging::LogState::GetLogger("LoadCharacterInventory");
    fb.Seek(offset);

    const auto itemCount = fb.GetUint8();
    const auto capacity = fb.GetUint16LE();
    logger.Spam() << " Items: " << +itemCount << " cap: " << capacity << "\n";
    return LoadInventory(fb, itemCount, capacity);
}

Conditions LoadConditions(FileBuffer& fb, unsigned character)
{
    ASSERT(character < SaveOffsets::sCharacterCount);
    fb.Seek(SaveOffsets::GetCharacterConditionOffset(character));

    auto conditions = Conditions{};
    for (unsigned i = 0; i < Conditions::sNumConditions; i++)
        conditions.mConditions[i] = fb.GetUint8();
    return conditions;
}

std::vector<SkillAffector> GetCharacterSkillAffectors(
    FileBuffer& fb,
    CharIndex character)
{
    fb.Seek(SaveOffsets::GetCharacterAffectorsOffset(character.mValue));
    std::vector<SkillAffector> affectors{};
    for (unsigned i = 0; i < 8; i++)
    {
        const auto type = fb.GetUint16LE();
        if (type == 0)
        {
            fb.Skip(12);
            continue;
        }
        const auto skill = ToSkill(static_cast<BAK::SkillTypeMask>(fb.GetUint16LE()));
        const auto adjust = fb.GetSint16LE();
        const auto startTime = Time{fb.GetUint32LE()};
        const auto endTime = Time{fb.GetUint32LE()};
        affectors.emplace_back(SkillAffector{type, skill, adjust, startTime, endTime});
    }
    return affectors;
}

void Save(const Character& c, FileBuffer& fb)
{
    const auto charIndex = c.mCharacterIndex.mValue;
    // Skills
    fb.Seek(BAK::SaveOffsets::GetCharacterSkillOffset(charIndex));
    fb.Skip(2); // Character name offset
    auto* spells = reinterpret_cast<const std::uint8_t*>(&c.GetSpells().GetSpellBytes());
    for (unsigned i = 0; i < 6; i++)
    {
        fb.PutUint8(spells[i]);
    }

    const auto& skills = c.GetSkills();
    for (unsigned i = 0; i < Skills::sSkills; i++)
    {
        const auto& skill  = skills.GetSkill(static_cast<BAK::SkillType>(i));
        fb.PutUint8(skill.mMax);
        fb.PutUint8(skill.mTrueSkill);
        fb.PutUint8(skill.mCurrent);
        fb.PutUint8(skill.mExperience);
        fb.PutUint8(skill.mModifier);

        const auto pos = fb.Tell();
        State::SetSkillSelected(fb, charIndex, i, skill.mSelected);
        State::SetSkillUnseenImprovement(fb, charIndex, i, skill.mUnseenImprovement);
        fb.Seek(pos);
    }

    // Inventory
    fb.Seek(BAK::SaveOffsets::GetCharacterInventoryOffset(charIndex));
    fb.PutUint8(c.GetInventory().GetNumberItems());
    fb.PutUint16LE(c.GetInventory().GetCapacity());
    Save(c.GetInventory(), fb);

    // Conditions
    fb.Seek(BAK::SaveOffsets::GetCharacterConditionOffset(charIndex));
    for (unsigned i = 0; i < Conditions::sNumConditions; i++)
    {
        const auto cond = c.mConditions.GetCondition(static_cast<BAK::Condition>(i));
        fb.PutUint8(cond.Get());
    }

    fb.Seek(BAK::SaveOffsets::GetCharacterAffectorsOffset(charIndex));
    for (const auto& affector : c.GetSkillAffectors())
    {
        fb.PutUint16LE(affector.mType);
        fb.PutUint16LE(1 << static_cast<std::uint16_t>(affector.mSkill));
        fb.PutSint16LE(affector.mAdjustment);
        fb.PutUint32LE(affector.mStartTime.mTime);
        fb.PutUint32LE(affector.mEndTime.mTime);
    }
}

}
