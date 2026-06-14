#pragma once

#include "bak/coordinates.hpp"
#include "bak/combat/combat.hpp"
#include "bak/types.hpp"

#include "game/combatModelLoader.hpp"
#include "game/systems.hpp"

namespace Game {

class ActiveCombatant {
public:
    BAK::EntityIndex mItemId;
    std::pair<unsigned, unsigned> mObject;
    glm::vec3 mLocation;
    glm::vec3 mRotation;
    glm::vec3 mScale;

    BAK::MonsterIndex mMonster;
    BAK::AnimationType mAnimationType;
    BAK::Direction mDirection;
    std::size_t mFrame;
    const CombatModelLoader& mCombatModelLoader;

    void Update()
    {
        auto request = AnimationRequest{mAnimationType, mDirection};
        const auto& datas = *mCombatModelLoader.mCombatModelDatas[mMonster.mValue];
        if (!datas.mOffsetMap.contains(request))
        {
            return;
        }
        auto animOff = datas.mOffsetMap.at(request);
        mFrame = mFrame % animOff.mFrames;
        mObject = datas.mObjectDrawData[animOff.mOffset + mFrame];
    }

    void SetState(BAK::AnimationType type, BAK::Direction direction)
    {
        mAnimationType = type;
        mDirection = direction;
        const auto& monster = *mCombatModelLoader.mCombatModels[mMonster.mValue];
        const auto& anim = monster.GetAnimation(type, direction);
        mFrame = anim.mImageIndices.size() - 1;
        Update();
    }

    const auto& GetRenderData() const
    {
        return mCombatModelLoader.mCombatModelDatas[mMonster.mValue]->mRenderData;
    }
};


class CombatantManager
{
public:
    CombatantManager(
        const CombatModelLoader& loader,
        Systems* systems)
    :
        mLoader{loader},
        mSystems{systems}
    {
        mCombatants.reserve(2048);
    }

    BAK::EntityIndex AddCombatant(
        BAK::GamePosition pos,
        BAK::MonsterIndex monsterIndex)
    {
        assert(mSystems);
        auto entityId = mSystems->GetNextItemId();
        assert(mLoader.mCombatModelDatas[monsterIndex.mValue]);
        const auto& datas = *mLoader.mCombatModelDatas[monsterIndex.mValue];
        const auto& monster = *mLoader.mCombatModels[monsterIndex.mValue];
        const auto deadFrameOffset = monster.GetAnimation(
            BAK::AnimationType::Dead,
            BAK::Direction::South).mImageIndices.size() - 1;

        mCombatants.emplace_back(
            ActiveCombatant{
                entityId,
                {},
                BAK::ToGlCoord<float>(pos),
                Graphics::sNinetyDegreeRotation,
                glm::vec3{1},
                monsterIndex,
                BAK::AnimationType::Idle,
                BAK::Direction::South,
                0,
                mLoader});

        return entityId;
    }

    ActiveCombatant* GetActiveCombatant(BAK::EntityIndex id)
    {
        auto it = std::find_if(mCombatants.begin(), mCombatants.end(), [id](auto c)
        {
            return c.mItemId == id;
        });

        if (it == mCombatants.end())
        {
            return nullptr;
        }

        return &(*it);
    }

    void SetSystems(Systems* systems)
    {
        mSystems = systems;
    }

    void clear()
    {
        for (auto& combatant : mCombatants)
        {
            mSystems->RemoveDynamicRenderable(combatant.mItemId);
        }
        mCombatants.clear();
    }

private:
    std::vector<ActiveCombatant> mCombatants{};
    const CombatModelLoader& mLoader;
    Systems* mSystems;

};

}
