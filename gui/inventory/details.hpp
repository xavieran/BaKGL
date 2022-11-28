#pragma once

#include "bak/dialogSources.hpp"
#include "bak/inventoryItem.hpp"

#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/centeredImage.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>


namespace Gui {

class Details : public Widget
{
public:
    Details(
        glm::vec2 pos,
        glm::vec2 dims,
        const Icons& icons,
        const Font& font)
    :
        Widget{
            RectTag{},
            glm::vec2{13, 11},
            glm::vec2{289, 121},
            Color::black,
            true
        },
        mIcons{icons},
        mFont{font},
        mName{
            glm::vec2{6, 3},
            glm::vec2{90, 40}
        },
        mItem{
            glm::vec2{8, 31},
            glm::vec2{80, 58}
        },
        mStatusText{
            glm::vec2{8, 27 + 58},
            glm::vec2{90, 22}
        },
        mDescriptionBackground{
            glm::vec2{98, 0},
            glm::vec2{188, 120},
            Color::buttonBackground,
            Color::buttonHighlight,
            Color::buttonShadow
        },
        mDescriptionText{
            glm::vec2{98, 0},
            glm::vec2{188, 120}
        },
        mMoreInfo{
            glm::vec2{8, 6 + 28 + 58 + 2 + 12},
            glm::vec2{80, 12},
            mFont,
            "#More Info",
            [this]{ ShowMoreInfo(); }
        }
    {
    }

    void AddItem(const BAK::InventoryItem& item)
    {
        const auto [ss, ti, dims] = mIcons.GetInventoryIcon(item.GetObject().mImageIndex);
        mItem.SetImage(ss, ti, dims);

        Logging::LogDebug(__FUNCTION__) << " Adding item: " << item << "\n";
        {
            std::stringstream ss{};
            ss << "#" << item.GetObject().mName << "\n";
            ss << "Condition: " << item.GetCondition() << "%";
            mName.AddText(mFont, ss.str(), true);
            Logging::LogDebug(__FUNCTION__) << " Name: " << ss.str() << "\n";
        }

        const auto description = std::string{BAK::DialogSources::GetItemDescription(item.GetItemIndex().mValue)};

        mDescriptionText.AddText(mFont, description, true, true, true);

        {
            std::stringstream ss{};
            bool comma{};
            ss << "#";

            if (item.IsBroken())
            {
                comma = true;
                ss << "Broken";
            }
            if (item.IsEquipped())
            {
                if (comma) ss << ", ";
                comma = true;
                ss << "Using";
            }
            if (item.IsRepairable())
            {
                if (comma) ss << ", ";
                comma = true;
                ss << "Repairable";
            }
            if (item.IsPoisoned())
            {
                if (comma) ss << ", ";
                comma = true;
                ss << "Poisoned";
            }

            Logging::LogDebug(__FUNCTION__) << " Status: " << ss.str();
            mStatusText.AddText(mFont, ss.str(), true, true);
        }

        AddChildren();
    }

private:
    void ShowMoreInfo()
    {
    }

    void AddChildren()
    {
        ClearChildren();
        AddChildBack(&mName);
        AddChildBack(&mItem);
        AddChildBack(&mDescriptionBackground);
        AddChildBack(&mDescriptionText);
        AddChildBack(&mStatusText);
        AddChildBack(&mMoreInfo);
    }

    const Icons& mIcons;
    const Font& mFont;
    TextBox mName;
    CenteredImage mItem;
    TextBox mStatusText;
    Button mDescriptionBackground;
    TextBox mDescriptionText;
    ClickButton mMoreInfo;
};

}
