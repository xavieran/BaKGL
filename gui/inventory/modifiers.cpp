#include "gui/inventory/modifiers.hpp"

#include "bak/inventoryItem.hpp"

#include "gui/icons.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

Modifiers::Modifiers(
    glm::vec2 pos,
    glm::vec2 dims,
    const Icons& icons,
    const BAK::InventoryItem& item)
:
    Widget{
        RectTag{},
        pos,
        dims,
        glm::vec4{},
        true
    },
    mIcons{icons},
    mItemRef{item},
    mModifiers{}
{
    UpdateModifiers();
    AddChildren();
}

void Modifiers::UpdateModifiers()
{
    mModifiers.clear();
    const auto mods = mItemRef.GetModifiers();
    glm::vec2 pos = glm::vec2{0, 0};

    if (mItemRef.IsPoisoned())
    {
        const auto [ss, ti, dims] = mIcons.GetInventoryModifierIcon(0);
            pos.x += dims.x + 2;
            mModifiers.emplace_back(
                ImageTag{},
                ss,
                ti,
                pos,
                dims,
            true);
    }

    for (const auto& mod : mods)
    {
        const auto [ss, ti, dims] = mIcons.GetInventoryModifierIcon(
            static_cast<unsigned>(mod) + 1);
        pos.x += dims.x + 2;
        mModifiers.emplace_back(
            ImageTag{},
            ss,
            ti,
            pos,
            dims,
            true);
    }

}

void Modifiers::AddChildren()
{
    ClearChildren();

    for (auto& mod : mModifiers)
        AddChildBack(&mod);
}

}
