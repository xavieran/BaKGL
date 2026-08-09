#pragma once

#include "bak/coordinates.hpp"

namespace Gui {

class IMainView
{
public:
    virtual void SetHeading(BAK::GameHeading heading) = 0;
    virtual void SetCanSaveBookmark(bool canSaveBookmark) = 0;
    virtual void SetFollowRoadVisible(bool visible) = 0;
    virtual void SetFollowRoadActive(bool active) = 0;
    virtual void SetZoomOutVisible(bool visible) = 0;
    virtual void SetZoomInVisible(bool visible) = 0;

    virtual ~IMainView() = default;
};

}
