#pragma once

#include "graphics/glm.hpp"
#include "graphics/types.hpp"
#include "graphics/guiTypes.hpp"

#include <glm/glm.hpp>

#include <functional>
#include <optional>
#include <ostream>
#include <vector>
#include <unordered_map>

namespace Graphics {

class IGuiElement
{
public:
    IGuiElement()
    :
        mChildren{}
    {}

    virtual const DrawInfo& GetDrawInfo() const = 0;
    virtual const PositionInfo& GetPositionInfo() const = 0;

    virtual const std::vector<IGuiElement*>& GetChildren() const
    {
        return mChildren;
    }

    virtual ~IGuiElement(){}

    void AddChildFront(Graphics::IGuiElement* elem)
    {
        mChildren.insert(mChildren.begin(), elem);
    }

    void AddChildBack(Graphics::IGuiElement* elem)
    {
        mChildren.emplace_back(elem);
    }

    std::vector<Graphics::IGuiElement*> mChildren;
};

std::ostream& operator<<(std::ostream& os, const IGuiElement& element)
{
    os  << "Element: { " << element.GetDrawInfo() << ", "
        << element.GetPositionInfo() << "}";
    return os;
}

}
