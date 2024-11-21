#pragma once

#include "gui/IAnimator.hpp"

#include <memory>
#include <vector>

#include <glm/glm.hpp>

namespace Logging {
class Logger;
}

namespace Gui {

class AnimatorStore
{
public:
    AnimatorStore();

    void AddAnimator(std::unique_ptr<IAnimator>&& animator);
    void OnTimeDelta(double delta);

private:
    std::vector<std::unique_ptr<IAnimator>> mAnimators;
    const Logging::Logger& mLogger;
};

}
