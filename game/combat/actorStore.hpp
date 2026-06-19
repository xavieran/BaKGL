#pragma once

#include "bak/coordinates.hpp"
#include "bak/combat/combat.hpp"
#include "bak/types.hpp"

#include "game/combatModelLoader.hpp"
#include "game/systems.hpp"

namespace Game {

class Actor {
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
    glm::mat4 mModelMatrix{1.0f};

    void CalculateModelMatrix()
    {
        mModelMatrix = Graphics::CalculateModelMatrix(mLocation, mScale, mRotation, BAK::gWorldScale);
    }

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
        CalculateModelMatrix();
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


class ActorStore
{
public:
    ActorStore(
        const CombatModelLoader& loader,
        Systems* systems)
    :
        mLoader{loader},
        mSystems{systems}
    {
        mActors.reserve(2048);
    }

    BAK::EntityIndex AddActor(
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

        mActors.emplace_back(
            Actor{
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
        mActors.back().Update();

        return entityId;
    }

    Actor* GetActor(BAK::EntityIndex id)
    {
        auto it = std::find_if(mActors.begin(), mActors.end(), [id](auto c)
        {
            return c.mItemId == id;
        });

        if (it == mActors.end())
        {
            return nullptr;
        }

        return &(*it);
    }

    void SetSystems(Systems* systems)
    {
        mSystems = systems;
    }

    void Clear()
    {
        for (auto& actor : mActors)
        {
            mSystems->RemoveDynamicRenderable(actor.mItemId);
        }
        mActors.clear();
    }

    auto& GetActors()
    {
        return mActors;
    }

private:
    std::vector<Actor> mActors{};
    const CombatModelLoader& mLoader;
    Systems* mSystems;

};

}
