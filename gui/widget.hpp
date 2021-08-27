#pragma once

#include "com/logger.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/guiTypes.hpp"
#include "graphics/glm.hpp"

namespace Gui {

class Widget : public Graphics::IGuiElement
{
public:
    Widget(
        Graphics::DrawMode drawMode,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex texture,
        Graphics::ColorMode colorMode,
        glm::vec4 color,
        glm::vec2 pos,
        glm::vec2 dims,
        bool childrenRelative);

    virtual void SetActive();
    virtual void SetInactive();

    virtual void LeftMousePress(glm::vec2 click);
    virtual void LeftMouseRelease(glm::vec2 click);
    virtual void RightMousePress(glm::vec2 click);
    virtual void RightMouseRelease(glm::vec2 click);
    virtual void MouseMoved(glm::vec2 pos);

    const Graphics::DrawInfo& GetDrawInfo() const override;
    const Graphics::PositionInfo& GetPositionInfo() const override;

    void AddChildFront(Widget* widget);
    void AddChildBack(Widget* widget);
    void RemoveChild(Widget* elem);
    void ClearChildren();

    void SetCenter(glm::vec2 pos);
    void SetPosition(glm::vec2 pos);
    void AdjustPosition(glm::vec2 adj);
    void SetDimensions(glm::vec2 dims);

    void SetTexture(Graphics::TextureIndex);

protected:
    bool Within(glm::vec2 click);
    glm::vec2 TransformEvent(glm::vec2 eventPos);

    Graphics::DrawInfo mDrawInfo;
    Graphics::PositionInfo mPositionInfo;
    std::vector<Widget*> mChildren;
    bool mActive;
};

}
