#pragma once

#include "bak/coordinates.hpp"
#include "bak/types.hpp"

#include "game/combatModelLoader.hpp"
#include "game/systems.hpp"

#include "com/random.hpp"

#include <optional>

namespace Game {

class Actor {
public:

    static constexpr auto sMinIdleTimeMs = 400;
    static constexpr auto sMaxIdleTimeMs = 800;

    BAK::EntityIndex mEntityId;
    std::pair<unsigned, unsigned> mObject;
    glm::vec3 mLocation;
    glm::vec3 mRotation;
    glm::vec3 mScale;

    BAK::MonsterIndex mMonster;
    BAK::AnimationType mAnimationType;
    BAK::Direction mDirection;
    std::optional<glm::vec4> mFlashColor{};

    const CombatModelLoader& mCombatModelLoader;
    glm::mat4 mModelMatrix{1.0f};
    bool mUpdateIdle{true};
    bool mAnimating{false};

    Actor(
        BAK::EntityIndex entityId,
        glm::vec3 location,
        BAK::MonsterIndex monster,
        const CombatModelLoader& loader)
    :
        mEntityId{entityId},
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
            auto frameCount = anim->mMeta.mFrames;
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

    void SetUpdateIdle(bool update)
    {
        mUpdateIdle = update;
    }

    void SetState(BAK::AnimationType type, BAK::Direction direction)
    {
        mAnimationType = type;
        mDirection = direction;
        if (auto anim = GetAnimData())
        {
            mFrame = anim->mMeta.mFrames > 0
                ? static_cast<int>(anim->mMeta.mFrames) - 1
                : 0;
            ApplyFrame(*anim);
        }
    }

    bool AdvanceAnimation()
    {
        mFrame++;
        if (auto anim = GetAnimData())
        {
            auto frameCount = anim->mMeta.mFrames;
            if (frameCount > 0 && mFrame >= static_cast<int>(frameCount))
            {
                mFrame = frameCount - 1;
                ApplyFrame(*anim);
                return true;
            }
            else if (frameCount <= 1)
            {
                mFrame = 0;
            }
            ApplyFrame(*anim);
        }
        return false;
    }

    void AdvanceIdleFrame()
    {
        auto anim = GetAnimData();
        if (!anim || anim->mMeta.mFrames <= 1)
        {
            return;
        }

        mFrame += mIdleDelta;
        auto maxFrame = static_cast<int>(anim->mMeta.mFrames) - 1;
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

    void RandomiseIdleFrame()
    {
        auto anim = GetAnimData();
        if (!anim || anim->mMeta.mFrames <= 1)
            return;

        mFrame = GetRandomNumber(0, anim->mMeta.mFrames - 1);
        mIdleAccumulator = GetRandomIdleInterval();
        mIdleInterval = GetRandomIdleInterval();
        ApplyFrame(*anim);
    }

    bool AdvanceIdle(double timeDelta)
    {
        if (mAnimating || !mUpdateIdle) return false;
        mIdleAccumulator += timeDelta;
        if (mIdleAccumulator < mIdleInterval) return false;
        mIdleAccumulator -= mIdleInterval;
        AdvanceIdleFrame();
        mIdleInterval = GetRandomIdleInterval();
        return true;
    }

    double GetRandomIdleInterval() const
    {
        return GetRandomNumber(sMinIdleTimeMs, sMaxIdleTimeMs) / 1000.0;
    }

    const auto& GetRenderData() const
    {
        return mCombatModelLoader.mCombatModelDatas[mMonster.mValue]->mRenderData;
    }

private:
    struct AnimData
    {
        const CombatModelData* mDatas;
        AnimationMeta mMeta;
    };

    std::optional<AnimData> GetAnimData() const
    {
        auto request = AnimationRequest{mAnimationType, BAK::ToSpriteDirection(mDirection)};
        auto& datas = mCombatModelLoader.mCombatModelDatas[mMonster.mValue];
        if (!datas || !datas->mOffsetMap.contains(request))
        {
            return std::nullopt;
        }
        return AnimData{&(*datas), datas->mOffsetMap.at(request)};
    }

    void ApplyFrame(const AnimData& anim)
    {
        auto animationOffset = anim.mMeta.mOffset + mFrame;
        if (animationOffset >= anim.mDatas->mObjectDrawData.size())
        {
            Logging::LogFatal(__FUNCTION__) << "Attempted to access animation outside of draw data size: "
                << ToString(mAnimationType) << " dir: " << ToString(mDirection) << " frame: " << mFrame << std::endl;
            assert(false);
        }
        mObject = anim.mDatas->mObjectDrawData[animationOffset];
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

    int mFrame{0};
    int mIdleDelta{1};
    double mIdleAccumulator{0};
    double mIdleInterval{0.5};
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
            return c.mEntityId == id;
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
            mSystems->RemoveDynamicRenderable(actor.mEntityId);
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
