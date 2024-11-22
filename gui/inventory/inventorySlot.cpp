#include "gui/inventory/inventorySlot.hpp"

#include "bak/inventoryItem.hpp"
#include "bak/objectInfo.hpp"

#include "gui/inventory/modifiers.hpp"

#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/mouseEvent.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

InventorySlot::InventorySlot(
    glm::vec2 pos,
    glm::vec2 dims,
    const Font& font,
    const Icons& icons,
    BAK::InventoryIndex itemIndex,
    const BAK::InventoryItem& item,
    std::function<void()>&& useItemDirectly,
    std::function<void()>&& showItemDescription)
:
    Widget{
        RectTag{},
        pos,
        dims,
        glm::vec4{},
        true
    },
    mItemIndex{itemIndex},
    mItemRef{item},
    mUseItemDirectly{std::move(useItemDirectly)},
    mShowItemDescription{std::move(showItemDescription)},
    mIsSelected{false},
    mQuantity{
        glm::vec2{0, 0},
        glm::vec2{40, 30}
    },
    mItem{
        ImageTag{},
        std::get<Graphics::SpriteSheetIndex>(
            icons.GetInventoryIcon(item.GetObject().mImageIndex
                + (item.IsActivated() ? 44 : 0))), // FIXME: This doesn't work for ring of prandur
        std::get<Graphics::TextureIndex>(
            icons.GetInventoryIcon(item.GetObject().mImageIndex
                + (item.IsActivated() ? 44 : 0))),
        pos,
        std::get<glm::vec2>(
            icons.GetInventoryIcon(item.GetObject().mImageIndex)),
        true
    },
    mModifiers{
        glm::vec2{0},
        glm::vec2{0},
        icons,
        item
    }
{
    assert(mShowItemDescription);
    mItem.SetCenter(GetCenter() - GetTopLeft());

    UpdateQuantity(font, item);

    AddChildren();
}

BAK::InventoryIndex InventorySlot::GetItemIndex() const
{
    return mItemIndex;
}

const BAK::InventoryItem& InventorySlot::GetItem() const
{
    return mItemRef;
}

bool InventorySlot::OnMouseEvent(const MouseEvent& event)
{
    const auto result = std::visit(overloaded{
        [this](const LeftMousePress& p){ return LeftMousePressed(p.mValue); },
        [this](const RightMousePress& p){ return RightMousePressed(p.mValue); },
        [](const auto& p){ return false; }
        },
        event);

    UpdateSelected();

    return result;
}

bool InventorySlot::LeftMousePressed(glm::vec2 click)
{
    if (Within(click))
    {
        if (mIsSelected)
        {
            mUseItemDirectly();
        }
        mIsSelected = true;
    }
    else
    {
        mIsSelected = false;
    }

    return false;
}

bool InventorySlot::RightMousePressed(glm::vec2 click)
{
    if (Within(click))
    {
        mIsSelected = true;
        mShowItemDescription();
    }
    else
        mIsSelected = false;

    return false;
}

void InventorySlot::UpdateSelected()
{
    if (mIsSelected)
        SetColor(Color::itemHighlighted);
    else
        SetColor(glm::vec4{});
}

void InventorySlot::UpdateQuantity(
    const Font& font,
    const BAK::InventoryItem& item)
{
    if (item.DisplayNumber())
    {
        std::stringstream ss{};
        ss << "#" << +item.GetCondition() << 
            (item.DisplayCondition()  ? "%" : "");
        const auto& [textDims, _] = mQuantity.SetText(font, ss.str());
        const auto& dims = GetPositionInfo().mDimensions;
        mQuantity.SetPosition(
            dims - textDims 
            + glm::vec2{4, 2});
    }
}

bool InventorySlot::IsSelected() const
{
    return mIsSelected;
}

void InventorySlot::ResetSelected()
{
    mIsSelected = false;
}

void InventorySlot::AddItem(bool snapToTop)
{
    if (snapToTop)
        mItem.SetPosition(
            glm::vec2{mItem.GetPositionInfo().mPosition.x, 8});
    AddChildBack(&mItem);
}

void InventorySlot::AddChildren()
{
    ClearChildren();
    AddItem(false);
    AddChildBack(&mQuantity);
    AddChildBack(&mModifiers);
}

}
