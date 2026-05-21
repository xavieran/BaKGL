#include "bak/lua/types.hpp"

#include "bak/bard.hpp"
#include "bak/condition.hpp"
#include "bak/entityType.hpp"
#include "bak/gameState.hpp"
#include "bak/IContainer.hpp"
#include "bak/inventoryItem.hpp"
#include "bak/objectInfo.hpp"
#include "bak/skills.hpp"

#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>

namespace BAK::Lua {

namespace {

template<typename T>
void RegisterEnum(lua_State* luaState, const char* name, std::initializer_list<std::pair<const char*, T>> values)
{
    lua_createtable(luaState, 0, values.size());
    for (auto& [key, val] : values)
    {
        lua_pushstring(luaState, key);
        lua_pushinteger(luaState, static_cast<lua_Integer>(val));
        lua_settable(luaState, -3);
    }
    lua_setglobal(luaState, name);
}

}

void RegisterTypes(lua_State* luaState)
{
    luabridge::getGlobalNamespace(luaState)
        .beginClass<BAK::Royals>("Royals")
            .addProperty("value", [](const BAK::Royals& r) -> unsigned { return r.mValue; })
        .endClass()
        .beginClass<BAK::Time>("Time")
            .addFunction("GetDays", &BAK::Time::GetDays)
            .addFunction("GetHour", &BAK::Time::GetHour)
            .addFunction("GetHours", &BAK::Time::GetHours)
            .addFunction("GetMinutes", &BAK::Time::GetMinutes)
            .addPropertyReadWrite("mTime", &BAK::Time::mTime)
        .endClass()
        .beginClass<BAK::WorldClock>("WorldClock")
            .addFunction("GetTime", &BAK::WorldClock::GetTime)
            .addFunction("GetTimeLastSlept", &BAK::WorldClock::GetTimeLastSlept)
            .addFunction("GetTimeSinceLastSlept", &BAK::WorldClock::GetTimeSinceLastSlept)
        .endClass()
        .beginClass<BAK::Skills>("Skills")
            .addFunction("GetSkill", [](BAK::Skills& skills, BAK::SkillType skill) -> unsigned {
                return skills.GetSkill(skill).mCurrent;
            })
        .endClass()
        .beginClass<BAK::Character>("Character")
            .addFunction("GetName", &BAK::Character::GetName)
            .addFunction("GetIndex", [](BAK::Character& character) { return character.GetIndex().mValue; })
            .addFunction("IsSpellcaster", &BAK::Character::IsSpellcaster)
            .addFunction("IsSwordsman", &BAK::Character::IsSwordsman)
            .addFunction("GetSkill", [](BAK::Character& character, BAK::SkillType skill) -> unsigned {
                return character.GetSkill(skill);
            })
            .addFunction("GetMaxSkill", [](BAK::Character& character, BAK::SkillType skill) -> unsigned {
                return character.GetMaxSkill(skill);
            })
            .addFunction("GetSkills", [](BAK::Character& character) -> BAK::Skills& {
                return character.GetSkills();
            })
        .endClass()
        .beginClass<BAK::Party>("Party")
            .addFunction("GetGold", [](BAK::Party& party) { return party.GetGold().mValue; })
            .addFunction("GetNumCharacters", &BAK::Party::GetNumCharacters)
            .addFunction("GetCharacter", [](BAK::Party& party, unsigned idx) -> BAK::Character& {
                return party.GetCharacter(BAK::CharIndex{idx});
            })
            .addFunction("GetSkill", [](BAK::Party& party, BAK::SkillType skill, bool best) -> unsigned {
                return party.GetSkill(skill, best).second;
            })
            .addFunction("GainMoney", [](BAK::Party& party, unsigned amount) {
                party.GainMoney(BAK::Royals{amount});
            })
            .addFunction("LoseMoney", [](BAK::Party& party, unsigned amount) {
                party.LoseMoney(BAK::Royals{amount});
            })
            .addFunction("SetMoney", [](BAK::Party& party, unsigned amount) {
                party.SetMoney(BAK::Royals{amount});
            })
            .addFunction("HaveItem", [](BAK::Party& party, unsigned item) {
                return party.HaveItem(BAK::ItemIndex{item});
            })
        .endClass()
        .beginClass<BAK::GameState>("GameState")
            .addFunction("GetMoney", [](BAK::GameState& gs) { return gs.GetMoney().mValue; })
            .addFunction("GetChapter", [](BAK::GameState& gs) { return gs.GetChapter().mValue; })
            .addFunction("GetParty", [](BAK::GameState& gs) -> BAK::Party& { return gs.GetParty(); })
            .addFunction("GetPartySkill", [](BAK::GameState& gs, BAK::SkillType skill, bool best) -> unsigned {
                return gs.GetPartySkill(skill, best).second;
            })
            .addFunction("GetZone", [](BAK::GameState& gs) { return gs.GetZone().mValue; })
            .addFunction("GetEndOfDialogState", &BAK::GameState::GetEndOfDialogState)
            .addFunction("DoSetEndOfDialogState", &BAK::GameState::DoSetEndOfDialogState)
            .addFunction("SetEventValue", &BAK::GameState::SetEventValue)
            .addFunction("SetEventState", &BAK::GameState::SetEventState)
            .addFunction("ReadEventBool", &BAK::GameState::ReadEventBool)
            .addFunction("GetWorldTime", [](BAK::GameState& gs) -> BAK::WorldClock& { return gs.GetWorldTime(); })
            .addFunction("GetPartyLeader", [](BAK::GameState& gs) { return gs.GetPartyLeader().mValue; })
            .addFunction("HealCharacter", &BAK::GameState::HealCharacter)
            .addFunction("SelectRandomActiveCharacter", &BAK::GameState::SelectRandomActiveCharacter)
            .addFunction("SetDialogContext_7530", &BAK::GameState::SetDialogContext_7530)
            .addFunction("SetActiveCharacter", [](BAK::GameState& gs, unsigned characterIndex) {
                gs.SetActiveCharacter(BAK::CharIndex{characterIndex});
            })
        .endClass();

    RegisterEnum<BAK::ItemType>(luaState, "ItemType", {
        {"Unspecified",      BAK::ItemType::Unspecified},
        {"Sword",            BAK::ItemType::Sword},
        {"Crossbow",         BAK::ItemType::Crossbow},
        {"Staff",            BAK::ItemType::Staff},
        {"Armor",            BAK::ItemType::Armor},
        {"Key",              BAK::ItemType::Key},
        {"Tool",             BAK::ItemType::Tool},
        {"WeaponOil",        BAK::ItemType::WeaponOil},
        {"ArmorOil",         BAK::ItemType::ArmorOil},
        {"SpecialOil",       BAK::ItemType::SpecialOil},
        {"Bowstring",        BAK::ItemType::Bowstring},
        {"Scroll",           BAK::ItemType::Scroll},
        {"Note",             BAK::ItemType::Note},
        {"Book",             BAK::ItemType::Book},
        {"Potion",           BAK::ItemType::Potion},
        {"Restoratives",     BAK::ItemType::Restoratives},
        {"ConditionModifier", BAK::ItemType::ConditionModifier},
        {"Light",            BAK::ItemType::Light},
        {"Ingredient",       BAK::ItemType::Ingredient},
        {"Ration",           BAK::ItemType::Ration},
        {"Food",             BAK::ItemType::Food},
        {"Other",            BAK::ItemType::Other},
    });

    RegisterEnum<BAK::SkillType>(luaState, "SkillType", {
        {"Health",      BAK::SkillType::Health},
        {"Stamina",     BAK::SkillType::Stamina},
        {"Speed",       BAK::SkillType::Speed},
        {"Strength",    BAK::SkillType::Strength},
        {"Defense",     BAK::SkillType::Defense},
        {"Crossbow",    BAK::SkillType::Crossbow},
        {"Melee",       BAK::SkillType::Melee},
        {"Casting",     BAK::SkillType::Casting},
        {"Assessment",  BAK::SkillType::Assessment},
        {"Armorcraft",  BAK::SkillType::Armorcraft},
        {"Weaponcraft", BAK::SkillType::Weaponcraft},
        {"Barding",     BAK::SkillType::Barding},
        {"Haggling",    BAK::SkillType::Haggling},
        {"Lockpick",    BAK::SkillType::Lockpick},
        {"Scouting",    BAK::SkillType::Scouting},
        {"Stealth",     BAK::SkillType::Stealth},
        {"TotalHealth", BAK::SkillType::TotalHealth},
    });

    RegisterEnum<BAK::Condition>(luaState, "Condition", {
        {"Sick",      BAK::Condition::Sick},
        {"Plagued",   BAK::Condition::Plagued},
        {"Poisoned",  BAK::Condition::Poisoned},
        {"Drunk",     BAK::Condition::Drunk},
        {"Healing",   BAK::Condition::Healing},
        {"Starving",  BAK::Condition::Starving},
        {"NearDeath", BAK::Condition::NearDeath},
    });

    RegisterEnum<BAK::Modifier>(luaState, "Modifier", {
        {"Flaming",      BAK::Modifier::Flaming},
        {"SteelFire",    BAK::Modifier::SteelFire},
        {"Frost",        BAK::Modifier::Frost},
        {"Enhancement1", BAK::Modifier::Enhancement1},
        {"Enhancement2", BAK::Modifier::Enhancement2},
        {"Blessing1",    BAK::Modifier::Blessing1},
        {"Blessing2",    BAK::Modifier::Blessing2},
        {"Blessing3",    BAK::Modifier::Blessing3},
    });

    RegisterEnum<BAK::RacialModifier>(luaState, "RacialModifier", {
        {"None",    BAK::RacialModifier::None},
        {"Tsurani", BAK::RacialModifier::Tsurani},
        {"Elf",     BAK::RacialModifier::Elf},
        {"Human",   BAK::RacialModifier::Human},
        {"Dwarf",   BAK::RacialModifier::Dwarf},
    });

    RegisterEnum<BAK::ItemFlags>(luaState, "ItemFlags", {
        {"Consumable",     BAK::ItemFlags::Consumable},
        {"MagicalItem",    BAK::ItemFlags::MagicalItem},
        {"Combat",         BAK::ItemFlags::Combat},
        {"SwordsmanItem",  BAK::ItemFlags::SwordsmanItem},
        {"NonCombatItem",  BAK::ItemFlags::NonCombatItem},
        {"Stackable",      BAK::ItemFlags::Stackable},
        {"ConditionBased", BAK::ItemFlags::ConditionBased},
        {"ChargeBased",    BAK::ItemFlags::ChargeBased},
        {"QuantityBased",  BAK::ItemFlags::QuantityBased},
    });

    RegisterEnum<BAK::ItemStatus>(luaState, "ItemStatus", {
        {"Activated",  BAK::ItemStatus::Activated},
        {"Broken",     BAK::ItemStatus::Broken},
        {"Repairable", BAK::ItemStatus::Repairable},
        {"Equipped",   BAK::ItemStatus::Equipped},
        {"Poisoned",   BAK::ItemStatus::Poisoned},
    });

    RegisterEnum<BAK::EntityType>(luaState, "EntityType", {
        {"TERRAIN",    BAK::EntityType::TERRAIN},
        {"EXTERIOR",   BAK::EntityType::EXTERIOR},
        {"BRIDGE",     BAK::EntityType::BRIDGE},
        {"INTERIOR",   BAK::EntityType::INTERIOR},
        {"HILL",       BAK::EntityType::HILL},
        {"TREE",       BAK::EntityType::TREE},
        {"CHEST",      BAK::EntityType::CHEST},
        {"DEADBODY1",  BAK::EntityType::DEADBODY1},
        {"FENCE",      BAK::EntityType::FENCE},
        {"GATE",       BAK::EntityType::GATE},
        {"BUILDING",   BAK::EntityType::BUILDING},
        {"TOMBSTONE",  BAK::EntityType::TOMBSTONE},
        {"SIGN",       BAK::EntityType::SIGN},
        {"TUNNEL1",    BAK::EntityType::TUNNEL1},
        {"PIT",        BAK::EntityType::PIT},
        {"DEADBODY2",  BAK::EntityType::DEADBODY2},
        {"DIRTPILE",   BAK::EntityType::DIRTPILE},
        {"CORN",       BAK::EntityType::CORN},
        {"FIRE",       BAK::EntityType::FIRE},
        {"ENTRANCE",   BAK::EntityType::ENTRANCE},
        {"GROVE",      BAK::EntityType::GROVE},
        {"FERN",       BAK::EntityType::FERN},
        {"DOOR",       BAK::EntityType::DOOR},
        {"CRYST",      BAK::EntityType::CRYST},
        {"ROCKPILE",   BAK::EntityType::ROCKPILE},
        {"BUSH1",      BAK::EntityType::BUSH1},
        {"BUSH2",      BAK::EntityType::BUSH2},
        {"BUSH3",      BAK::EntityType::BUSH3},
        {"SLAB",       BAK::EntityType::SLAB},
        {"STUMP",      BAK::EntityType::STUMP},
        {"WELL",       BAK::EntityType::WELL},
        {"ENGINE",     BAK::EntityType::ENGINE},
        {"SCARECROW",  BAK::EntityType::SCARECROW},
        {"TRAP",       BAK::EntityType::TRAP},
        {"CATAPULT",   BAK::EntityType::CATAPULT},
        {"COLUMN",     BAK::EntityType::COLUMN},
        {"LANDSCAPE",  BAK::EntityType::LANDSCAPE},
        {"TUNNEL2",    BAK::EntityType::TUNNEL2},
        {"BAG",        BAK::EntityType::BAG},
        {"LADDER",     BAK::EntityType::LADDER},
        {"DEAD_COMBATANT",   BAK::EntityType::DEAD_COMBATANT},
        {"LIVING_COMBATANT", BAK::EntityType::LIVING_COMBATANT},
    });

    RegisterEnum<BAK::ContainerType>(luaState, "ContainerType", {
        {"Bag",           BAK::ContainerType::Bag},
        {"CT1",           BAK::ContainerType::CT1},
        {"Gravestone",    BAK::ContainerType::Gravestone},
        {"Building",      BAK::ContainerType::Building},
        {"Shop",          BAK::ContainerType::Shop},
        {"Inn",           BAK::ContainerType::Inn},
        {"TimirianyaHut", BAK::ContainerType::TimirianyaHut},
        {"Combat",        BAK::ContainerType::Combat},
        {"Chest",         BAK::ContainerType::Chest},
        {"FairyChest",    BAK::ContainerType::FairyChest},
        {"EventChest",    BAK::ContainerType::EventChest},
        {"Hole",          BAK::ContainerType::Hole},
        {"Key",           BAK::ContainerType::Key},
        {"Inv",           BAK::ContainerType::Inv},
    });

    RegisterEnum<BAK::Bard::BardStatus>(luaState, "BardStatus", {
        {"Failed", BAK::Bard::BardStatus::Failed},
        {"Poor",   BAK::Bard::BardStatus::Poor},
        {"Good",   BAK::Bard::BardStatus::Good},
        {"Best",   BAK::Bard::BardStatus::Best},
    });
}

}
