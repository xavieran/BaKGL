#pragma once

#include <functional>

namespace Gui {

// Interface for a DialogScene
// This should be implemented by GDSScenes and the 3d adventure view
class IDialogScene
{
public:
    virtual void DisplayNPCBackground() = 0;
    virtual void DisplayPlayerBackground() = 0;
    virtual void DialogFinished() = 0;
};

class NullDialogScene : public IDialogScene
{
public:
    void DisplayNPCBackground() override {}
    void DisplayPlayerBackground() override {}
    void DialogFinished() override {}
};

class DynamicDialogScene : public IDialogScene
{
public:
    DynamicDialogScene(
        std::function<void()>&& displayNPC,
        std::function<void()>&& displayPlayer,
        std::function<void()>&& dialogFinished)
    :
        mDisplayNPC{displayNPC},
        mDisplayPlayer{displayPlayer},
        mDialogFinished{dialogFinished}
    {}

    void DisplayNPCBackground() override { mDisplayNPC(); }
    void DisplayPlayerBackground() override { mDisplayPlayer(); }
    void DialogFinished() override { mDialogFinished(); }

private:
    std::function<void()> mDisplayNPC;
    std::function<void()> mDisplayPlayer;
    std::function<void()> mDialogFinished;
};

}
