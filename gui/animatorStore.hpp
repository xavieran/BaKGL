#pragma once

#include "com/assert.hpp"
#include "com/logger.hpp"
#include "com/visit.hpp"

#include "gui/animator.hpp"

#include <glm/glm.hpp>


namespace Gui {

class AnimatorStore
{
public:
    AnimatorStore()
    :
        mAnimators{},
        mLogger{Logging::LogState::GetLogger("Gui::AnimatorStore")}
    {
        mAnimators.reserve(20);
    }

    template <typename ...Args>
    void AddAnimator(Args&&... args)
    {
        mAnimators.emplace_back(std::forward<Args>(args)...);
        mLogger.Spam() << "Added animator @" << &mAnimators.back() << "\n";
    }

    void OnTimeDelta(double delta)
    {
        mLogger.Spam() << "Ticking : " << delta << "\n";
        for (auto& animator : mAnimators)
            animator.OnTimeDelta(delta);

        mAnimators.erase(
            std::remove_if(
                mAnimators.begin(), mAnimators.end(),
                [&](const auto& a){
                    return !a.IsAlive();
                }),
            mAnimators.end());
    }

private:
    std::vector<LinearAnimator> mAnimators;
    const Logging::Logger& mLogger;
};

}
