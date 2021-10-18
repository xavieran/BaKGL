#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace BAK {

class Layout
{
public:
    struct LayoutWidget
    {
        unsigned mWidget;
        unsigned mAction;
        bool mIsVisibile;
        glm::vec2 mPosition;
        glm::vec2 mDimensions;
        unsigned mTeleport;
        unsigned mImage;
        unsigned mGroup;
        std::string mLabel;
    };

    Layout(
        const std::string& path);

    const LayoutWidget& GetWidget(unsigned index) const;
    glm::vec2 GetWidgetLocation(unsigned index) const;
    glm::vec2 GetWidgetDimensions(unsigned index) const;

    std::size_t GetSize() const { return mWidgets.size(); }

private:
    bool mIsPopup;
    glm::vec2 mPosition;
    glm::vec2 mDimensions;

    glm::vec2 mOffset;

    std::vector<LayoutWidget> mWidgets;
};

}
