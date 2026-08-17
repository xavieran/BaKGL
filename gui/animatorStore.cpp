#include "gui/animatorStore.hpp"

#include "com/logger.hpp"

namespace Gui {

AnimatorStore::AnimatorStore()
:
    mAnimators{},
    mLogger{Logging::LogState::GetLogger("Gui::AnimatorStore")}
{
}

void AnimatorStore::AddAnimator(std::unique_ptr<IAnimator>&& animator)
{
    mAnimators.emplace_back(std::move(animator));
    mLogger.Spam() << "Added animator @" << mAnimators.back() << "\n";
}

void AnimatorStore::OnTimeDelta(double delta)
{
    mLogger.Spam() << "Ticking : " << delta << "\n";
    const auto ticked = mAnimators.size();
    for (unsigned i = 0; i < ticked; i++)
        mAnimators[i]->OnTimeDelta(delta);

    mAnimators.erase(
        std::remove_if(
            mAnimators.begin(), mAnimators.end(),
            [&](const auto& a){
                return !a->IsAlive();
            }),
        mAnimators.end());
}

}
