#include "graphics/IGuiElement.hpp"

namespace Graphics {

IGuiElement::IGuiElement()
:
    mChildren{}
{}

const std::vector<IGuiElement*>& 
IGuiElement::GetChildren() const
{
    return mChildren;
}

IGuiElement::~IGuiElement()
{

}

void IGuiElement::AddChildFront(Graphics::IGuiElement* elem)
{
    mChildren.insert(mChildren.begin(), elem);
}

void IGuiElement::AddChildBack(Graphics::IGuiElement* elem)
{
    mChildren.emplace_back(elem);
}

void IGuiElement::RemoveChild(Graphics::IGuiElement* elem)
{
    const auto it = std::find(mChildren.begin(), mChildren.end(), elem);
    assert(it != mChildren.end());
    mChildren.erase(it);
}

void IGuiElement::ClearChildren()
{
    mChildren.clear();
}

std::ostream& operator<<(std::ostream& os, const IGuiElement& element)
{
os  << "Element: { " << element.GetDrawInfo() << ", "
    << element.GetPositionInfo() << "}";
return os;
}

}
