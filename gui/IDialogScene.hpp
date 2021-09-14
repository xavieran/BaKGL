#pragma once

namespace Gui {

// Interface for a DialogScene
// This should be implemented by GDSScenes and the 3d adventure view
class IDialogScene
{
public:
    virtual void DisplayNPCBackground() = 0;
    virtual void DisplayPlayerBackground() = 0;
};

class NullDialogScene : public IDialogScene
{
public:
    void DisplayNPCBackground() override {}
    void DisplayPlayerBackground() override {}
};

}
