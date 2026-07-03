#pragma once

#include "bak/partyChangeCache.hpp"
#include "bak/character.hpp"
#include "bak/combat/combat.hpp"
#include "bak/container.hpp"
#include "bak/dialogAction.hpp"
#include "bak/dialogChoice.hpp"
#include "bak/encounter/encounter.hpp"
#include "bak/fmap.hpp"
#include "bak/gameData.hpp"
#include "bak/inventoryItem.hpp"
#include "bak/spells.hpp"
#include "bak/textVariableStore.hpp"
#include "bak/timeExpiringState.hpp"
#include "bak/types.hpp"

#include "com/random.hpp"
#include "com/visit.hpp"

#include <functional>
#include <memory>
#include <type_traits>

namespace BAK {

class GameData;

namespace Encounter {
class Encounter;
}

template <typename F, typename ...Args>
using invoke_result_with_fb = std::invoke_result_t<F, FileBuffer&, Args...>;

class GameState
{
public:
    GameState();

    template <typename F, typename ...Args>
    void Apply(F&& func, Args&&... args) const
        requires(std::is_void_v<invoke_result_with_fb<F, Args...>>)
    {
        if (mGameData.IsLoaded())
            std::invoke(func, mGameData.GetFileBuffer(), args...);
    }

    template <typename F, typename ...Args>
    void Apply(F&& func, Args&&... args)
        requires(std::is_void_v<invoke_result_with_fb<F, Args...>>)
    {
        if (mGameData.IsLoaded())
            std::invoke(func, mGameData.GetFileBuffer(), args...);
    }

    template <typename F, typename ...Args>
    auto Apply(F&& func, Args&&... args)
        requires(!std::is_void_v<invoke_result_with_fb<F, Args...>>)
    {
        if (mGameData.IsLoaded())
            return std::invoke(func, mGameData.GetFileBuffer(), args...);
        return invoke_result_with_fb<F, Args...>{};
    }

    template <typename F, typename ...Args>
    auto Apply(F&& func, Args&&... args) const
        requires(!std::is_void_v<invoke_result_with_fb<F, Args...>>)
    {
        if (mGameData.IsLoaded())
            return std::invoke(func, mGameData.GetFileBuffer(), args...);
        return invoke_result_with_fb<F, Args...>{};
    }

    GameData& GetGameData() { return mGameData; }
    const GameData& GetGameData() const { return mGameData; }

    void LoadGame(std::string savePath);

    const Party& GetParty() const;
    Party& GetParty();

    std::int16_t GetEndOfDialogState() const;
    void DoSetEndOfDialogState(std::int16_t state);

    void SetActiveCharacter(ActiveCharIndex character);
    void SetActiveCharacter(CharIndex character);
    
    const TextVariableStore& GetTextVariableStore() const;
    TextVariableStore& GetTextVariableStore();

    void SetChapter(Chapter chapter);
    Chapter GetChapter() const;

    void SetMonster(MonsterIndex);

    Royals GetMoney() const;

    void SetLocation(Location loc);
    void SetLocation(GamePositionAndHeading posAndHeading);
    void SetMapLocation(MapLocation);
    GamePositionAndHeading GetLocation() const;
    MapLocation GetMapLocation() const;
    ZoneNumber GetZone() const;

    auto& GetTimeExpiringState() { return mTimeExpiringState; }

    const WorldClock& GetWorldTime() const;
    WorldClock& GetWorldTime();

    void SetTransitionChapter_7541(bool);
    bool GetTransitionChapter_7541() const;
    void SetShopType_7542(unsigned shopType);
    unsigned GetShopType_7542() const;

    GenericContainer* GetContainerForGDSScene(HotspotRef ref);
    GenericContainer* GetWorldContainer(ZoneNumber zone, GamePosition location);
    GenericContainer const* GetWorldContainer(ZoneNumber zone, GamePosition location) const;

    std::optional<unsigned> GetActor(unsigned actor) const;

    bool GetSpellActive(StaticSpells spell) const;
    bool ClearActiveSpells();

    // prefer to use this function when getting best skill
    // as it will set the appropriate internal state.
    std::pair<CharIndex, unsigned> GetPartySkill(SkillType skill, bool best);

    void SetCharacterTextVariables();
    void SetDefaultDialogTextVariables();
    void SetDialogTextVariable(unsigned index, unsigned attribute);

    CharIndex GetPartyLeader() const;

    void SelectRandomActiveCharacter(unsigned index, unsigned attribute);

    using FindEncounterCallback = std::function<const Encounter::Encounter&(BAK::CombatIndex)>;
    void SetFindEncounterCallback(FindEncounterCallback callback);

    void EvaluateAction(const DialogAction& action);
    void EvaluateSpecialAction(const SpecialAction& action);
    void DoGamble(unsigned playerChance, unsigned gamblerChance, unsigned reward);
    unsigned GetState(const Choice& choice) const;
    unsigned GetGameState(const GameStateChoice& choice) const;
    bool EvaluateGameStateChoice(const GameStateChoice& choice) const;

    void DoElapseTime(Time time);
    void ReduceAndEvaluateTimeExpiringState(Time delta);
    void RecalculatePalettesForPotionOrSpellEffect(TimeExpiringState& state);
    void DeactivateLightSource();
    bool HaveActiveLightSource();

    void SetSpellState(const TimeExpiringState& state);
    void CastStaticSpell(StaticSpells spell, Time duration);
    bool CanCastSpell(SpellIndex spell, ActiveCharIndex character);

    void HealCharacter(CharIndex who, unsigned amount);

    // Remove these from game state, now that we have Apply fn that can be used instead
    bool EvaluateDialogChoice(const DialogChoice& choice) const;

    unsigned GetEventState(Choice choice) const;
    unsigned ReadEvent(unsigned eventPtr) const;
    bool ReadEventBool(unsigned eventPtr) const;
    void SetEventValue(unsigned eventPtr, unsigned value);
    void SetEventState(const SetFlag& setFlag);
    bool GetMoreThanOneTempleSeen() const;

    void SetDialogContext_7530(unsigned contextValue);
    void SetImprovedSkill(SkillType skill);
    void SetBardReward_754d(unsigned value);
    unsigned GetBardReward_754d();
    void SetItemValue(Royals value);
    void SetInventoryItem(const InventoryItem& item);
    void ClearUnseenImprovements(unsigned character);

    bool HaveNote(unsigned note) const;
    bool CheckConversationItemAvailable(unsigned conversationItem) const;
    
    bool SaveState();
    bool Save(const SaveFile& saveFile);
    bool Save(const std::string& saveName);

    std::vector<GenericContainer>& GetContainers(ZoneNumber zone);
    const std::vector<GenericContainer>& GetContainers(ZoneNumber zone) const;
    std::vector<GenericContainer>& GetCombatContainers() { return mCombatContainers; }
    std::vector<CombatWorldLocation>& GetCombatWorldLocations() { return mCombatWorldLocations; }
    CombatWorldLocation& GetCombatWorldLocation(
        std::uint8_t tileIndex,
        unsigned tileCombatIndex,
        std::uint8_t combatantRelativeIndex);
    CombatantGridLocation& GetCombatantGridLocation(CombatantIndex);
    Character* GetCombatantCharacter(CombatantIndex);
    CombatEntityList& GetCombatEntityList(CombatIndex);
    std::optional<CombatRelInfo> GetCombatRelInfo(CombatantIndex combatant) const;
    GenericContainer* GetCombatContainer(CombatRelInfo);
    void ReactivateCombat(const Encounter::Encounter&, CombatIndex);

    // Super lame, but BaK uses a global so I will too
    void SetCombatTriggeredFromInteractable(bool value) { mCombatTriggeredFromInteractable = value; }
    bool GetCombatTriggeredFromInteractable() const { return mCombatTriggeredFromInteractable; }

    PartyChangeCache& GetPartyChangeCache() { return mPartyChangeCache; }

private:
    std::vector<CombatEntityList> RegenerateCombatEntityLists();

    std::optional<CharIndex> mDialogCharacter{};
    CharIndex mActiveCharacter{};
    CharIndex mSkillCheckedCharacter{};

    // 0 - defaults to someone other than party leader
    // 1 - will be selected so as not to equal 0
    // 2 - will be selected so as not to be either 0 or 1
    // 3 - party magician
    // 4 - party leader
    // 5 - party warrior
    std::array<std::uint8_t, 6> mDialogCharacterList{};

    GameData mGameData;
    FindEncounterCallback mFindEncounterCallback;
    unsigned mContextValue_7530{};
    bool mTransitionChapter_7541{};
    unsigned mShopType_7542{};
    unsigned mBardReward_754d{};
    SkillType mRecentlyImprovedSkill{};
    Royals mItemValue_753e{};
    unsigned mSkillValue{};
    unsigned mContextVar_753f{};
    PartyChangeCache mPartyChangeCache{};
    bool mCombatTriggeredFromInteractable{};

    std::optional<InventoryItem> mSelectedItem{};
    std::optional<MonsterIndex> mCurrentMonster{};
    std::int16_t mEndOfDialogState{};
    std::vector<
        std::vector<GenericContainer>> mContainers{};
    std::vector<GenericContainer> mGDSContainers{};
    std::vector<GenericContainer> mCombatContainers;
    std::vector<TimeExpiringState> mTimeExpiringState{};
    std::vector<CombatEntityList> mCombatEntityLists{};
    std::vector<CombatWorldLocation> mCombatWorldLocations{};
    std::vector<CombatantGridLocation> mCombatantGridLocations{};
    std::vector<Character> mCombatCharacters{};
    std::vector<CombatantGridLocation> mPlayerGridLocations{};
    SpellState mSpellState{};
    TextVariableStore mTextVariableStore{};
    FMapXY mFullMap;
    const Logging::Logger& mLogger;
};

}
