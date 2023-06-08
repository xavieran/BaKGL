#pragma once

namespace Gui {

class IAnimator
{
public:
    virtual void OnTimeDelta(double delta) = 0;
    virtual bool IsAlive() const = 0;

    virtual ~IAnimator() = default;
};

}
