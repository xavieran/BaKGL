#pragma once

namespace BAK
{

class ICameraManager
{
public:
    virtual void ToggleFollowRoad() = 0;
    virtual void ShowOverheadView() = 0;
    virtual void ShowFirstPersonView() = 0;
    virtual void ZoomOut() = 0;
    virtual void ZoomIn() = 0;

    virtual ~ICameraManager() = default;
};

}
