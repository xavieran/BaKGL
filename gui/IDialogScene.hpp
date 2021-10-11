#pragma once

#include "bak/types.hpp"

#include <functional>
#include <optional>

namespace Gui {

// Interface for a DialogScene
// This should be implemented by GDSScenes and the 3d adventure view
class IDialogScene
{
public:
    virtual void DisplayNPCBackground() = 0;
    virtual void DisplayPlayerBackground() = 0;
    virtual void DialogFinished(const std::optional<BAK::ChoiceIndex>&) = 0;
};

class NullDialogScene : public IDialogScene
{
public:
    void DisplayNPCBackground() override {}
    void DisplayPlayerBackground() override {}
    void DialogFinished(const std::optional<BAK::ChoiceIndex>&) override {}
};

class DynamicDialogScene : public IDialogScene
{
public:
    DynamicDialogScene(
        std::function<void()>&& displayNPC,
        std::function<void()>&& displayPlayer,
        std::function<void(const std::optional<BAK::ChoiceIndex>&)>&& dialogFinished)
    :
        mDisplayNPC{displayNPC},
        mDisplayPlayer{displayPlayer},
        mDialogFinished{dialogFinished}
    {}

    void DisplayNPCBackground() override { mDisplayNPC(); }
    void DisplayPlayerBackground() override { mDisplayPlayer(); }
    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice) override { mDialogFinished(choice); }

    void SetDialogFinished(std::function<void(const std::optional<BAK::ChoiceIndex>&)>&& fn)
    {
        mDialogFinished = fn;
    }

    void ResetDialogFinished()
    {
        mDialogFinished = [](const auto&){};
    }

private:
    std::function<void()> mDisplayNPC;
    std::function<void()> mDisplayPlayer;
    std::function<void(const std::optional<BAK::ChoiceIndex>&)> mDialogFinished;
};

}
