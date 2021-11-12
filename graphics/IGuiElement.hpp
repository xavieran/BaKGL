#pragma once

#include "graphics/guiTypes.hpp"

#include <glm/glm.hpp>

#include <ostream>
#include <vector>

namespace Graphics {

class IGuiElement
{
public:
    IGuiElement();
    virtual ~IGuiElement();

    virtual const DrawInfo& GetDrawInfo() const = 0;
    virtual const PositionInfo& GetPositionInfo() const = 0;

    virtual const std::vector<IGuiElement*>& GetChildren() const;

    void AddChildFront(Graphics::IGuiElement* elem);
    void AddChildBack(Graphics::IGuiElement* elem);
    void RemoveChild(Graphics::IGuiElement* elem);
    void ClearChildren();

private:
    std::vector<Graphics::IGuiElement*> mChildren;
};

std::ostream& operator<<(std::ostream& os, const IGuiElement& element);

}
