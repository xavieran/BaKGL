#pragma once

#include "bak/hotspot.hpp"

namespace Gui {

class IGuiManager
{
public:
    virtual void EnterGDSScene(const BAK::HotspotRef&) = 0;
    virtual void ExitGDSScene() = 0;
};

class NullGuiManager : public IGuiManager
{
    void EnterGDSScene(const BAK::HotspotRef&) override {}
    void ExitGDSScene() override {}
};

}
