#pragma once

#include "bak/coordinates.hpp"
#include "bak/types.hpp"

#include "game/combatModelLoader.hpp"
#include "game/systems.hpp"

#include <optional>

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

    const CombatModelLoader& mCombatModelLoader;
    glm::mat4 mModelMatrix{1.0f};
    bool mAnimating{false};

    Actor(
        BAK::EntityIndex itemId,
        glm::vec3 location,
        BAK::MonsterIndex monster,
        const CombatModelLoader& loader)
    :
        mItemId{itemId},
        mLocation{location},
        mRotation{Graphics::sNinetyDegreeRotation},
        mScale{glm::vec3{1}},
        mMonster{monster},
        mAnimationType{BAK::AnimationType::Idle},
        mDirection{BAK::Direction::South},
        mCombatModelLoader{loader}
    {
    }

    void SetPosition(glm::vec3 pos)
    {
        mLocation = pos;
        CalculateModelMatrix();
    }

    void Update()
    {
        if (auto anim = GetAnimData())
        {
            auto frameCount = anim->offset.mFrames;
            mFrame = frameCount == 0 ? 0 : mFrame % frameCount;
            ApplyFrame(*anim);
        }
    }

    void StartAnimation(BAK::AnimationType type)
    {
        mAnimationType = type;
        mFrame = 0;
        Update();
    }

    void SetDirection(BAK::Direction direction)
    {
        mDirection = direction;
        Update();
    }

    void SetState(BAK::AnimationType type, BAK::Direction direction)
    {
        mAnimationType = type;
        mDirection = direction;
        if (auto anim = GetAnimData())
        {
            mFrame = anim->offset.mFrames > 0
                ? static_cast<int>(anim->offset.mFrames) - 1
                : 0;
            ApplyFrame(*anim);
        }
    }

    bool AdvanceAnimation()
    {
        mFrame++;
        if (auto anim = GetAnimData())
        {
            auto frameCount = anim->offset.mFrames;
            if (frameCount > 0)
                mFrame %= frameCount;
            ApplyFrame(*anim);
        }
        return mFrame == 0;
    }

    void AdvanceIdleFrame()
    {
        auto anim = GetAnimData();
        if (!anim || anim->offset.mFrames <= 1)
            return;

        mFrame += mIdleDelta;
        auto maxFrame = static_cast<int>(anim->offset.mFrames) - 1;
        if (mFrame > maxFrame)
        {
            mFrame = maxFrame - 1;
            mIdleDelta = -1;
        }
        else if (mFrame < 0)
        {
            mFrame = 1;
            mIdleDelta = 1;
        }
        ApplyFrame(*anim);
    }

    const auto& GetRenderData() const
    {
        return mCombatModelLoader.mCombatModelDatas[mMonster.mValue]->mRenderData;
    }

private:
    int mFrame{0};
    int mIdleDelta{1};

    struct AnimData {
        const CombatModelData* datas;
        AnimationOffset offset;
    };

    std::optional<AnimData> GetAnimData() const
    {
        auto request = AnimationRequest{mAnimationType, BAK::ToSpriteDirection(mDirection)};
        auto& datas = mCombatModelLoader.mCombatModelDatas[mMonster.mValue];
        if (!datas || !datas->mOffsetMap.contains(request))
            return std::nullopt;
        return AnimData{&(*datas), datas->mOffsetMap.at(request)};
    }

    void ApplyFrame(const AnimData& anim)
    {
        mObject = anim.datas->mObjectDrawData[anim.offset.mOffset + mFrame];
        CalculateModelMatrix();
    }

    void CalculateModelMatrix()
    {
        auto adjustedRotation = mRotation;
        if (BAK::ToSpriteDirection(mDirection) != mDirection)
        {
            adjustedRotation = -mRotation;
        }
        mModelMatrix = Graphics::CalculateModelMatrix(
            mLocation, mScale, adjustedRotation, BAK::gWorldScale);
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

        mActors.emplace_back(
            entityId,
            BAK::ToGlCoord<float>(pos),
            monsterIndex,
            mLoader);
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
