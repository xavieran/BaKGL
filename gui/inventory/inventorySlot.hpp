#pragma once

#include "bak/dialogSources.hpp"
#include "bak/inventory.hpp"
#include "bak/layout.hpp"
#include "bak/textureFactory.hpp"

#include "gui/inventory/modifiers.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/dragEndpoint.hpp"
#include "gui/draggable.hpp"
#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class InventorySlot;

using DraggableItem = Draggable<InventorySlot>;

template <typename Base>
using ItemEndpoint = DragEndpoint<
    Base,
    InventorySlot>;

class InventorySlot :
    public Widget
{
public:
    InventorySlot(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& font,
        const Icons& icons,
        BAK::InventoryIndex itemIndex,
        const BAK::InventoryItem& item,
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
        mShowItemDescription{std::move(showItemDescription)},
        mIsSelected{false},
        mQuantity{
            glm::vec2{0, 0},
            glm::vec2{40, 30}
        },
        mItem{
            ImageTag{},
            std::get<Graphics::SpriteSheetIndex>(
                icons.GetInventoryIcon(item.GetObject().mImageIndex)),
            std::get<Graphics::TextureIndex>(
                icons.GetInventoryIcon(item.GetObject().mImageIndex)),
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

    BAK::InventoryIndex GetItemIndex() const
    {
        return mItemIndex;
    }

    const BAK::InventoryItem& GetItem() const
    {
        return mItemRef;
    }

    bool OnMouseEvent(const MouseEvent& event) override
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

    bool LeftMousePressed(glm::vec2 click)
    {
        if (Within(click))
        {

            mIsSelected = true;
            Logging::LogDebug("InventoryItem") << "Clicked: " << mItemRef << "\n"
                << mItemRef.GetObject() << "\n";
        }
        else
        {
            mIsSelected = false;
        }

        return false;
    }

    bool RightMousePressed(glm::vec2 click)
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

    void UpdateSelected()
    {
        if (mIsSelected)
            SetColor(Color::itemHighlighted);
        else
            SetColor(glm::vec4{});
    }

    void UpdateQuantity(
        const Font& font,
        const BAK::InventoryItem& item)
    {
        if (item.DisplayNumber())
        {
            std::stringstream ss{};
            ss << "#" << +item.GetCondition() << 
                (item.DisplayCondition()  ? "%" : "");
            const auto& [textDims, _] = mQuantity.AddText(font, ss.str());
            const auto& dims = GetPositionInfo().mDimensions;
            mQuantity.SetPosition(
                dims - textDims 
                + glm::vec2{4, 2});
        }
    }

    bool IsSelected() const
    {
        return mIsSelected;
    }

    void ResetSelected()
    {
        mIsSelected = false;
    }

protected:
    void AddItem(bool snapToTop)
    {
        if (snapToTop)
            mItem.SetPosition(
                glm::vec2{mItem.GetPositionInfo().mPosition.x, 8});
        AddChildBack(&mItem);
    }

private:
    void AddChildren()
    {
        ClearChildren();
        AddItem(false);
        AddChildBack(&mQuantity);
        AddChildBack(&mModifiers);
    }
    
    const BAK::InventoryIndex mItemIndex;
    const BAK::InventoryItem& mItemRef;
    std::function<void()> mShowItemDescription;
    bool mIsSelected;

    TextBox mQuantity;
    Widget mItem;
    Modifiers mModifiers;
};

}
