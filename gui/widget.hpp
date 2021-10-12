#pragma once

#include "com/logger.hpp"

#include "gui/mouseEvent.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/guiTypes.hpp"
#include "graphics/glm.hpp"

namespace Gui {

struct ClipRegionTag {};
struct ImageTag {};
struct RectTag {};

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

    Widget(
        ImageTag,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex texture,
        glm::vec2 pos,
        glm::vec2 dims,
        bool childrenRelative);

    Widget(
        ClipRegionTag,
        glm::vec2 pos,
        glm::vec2 dims,
        bool childrenRelative);

    Widget(
        RectTag,
        glm::vec2 pos,
        glm::vec2 dims,
        glm::vec4 color,
        bool childrenRelative);

    virtual ~Widget();

    virtual void SetActive();
    virtual void SetInactive();

    [[nodiscard]] virtual bool OnMouseEvent(
        const MouseEvent& event);

    const Graphics::DrawInfo& GetDrawInfo() const override;
    const Graphics::PositionInfo& GetPositionInfo() const override;

    void AddChildFront(Widget* widget);
    void AddChildBack(Widget* widget);
    void RemoveChild(Widget* elem);
    void PopChild();
    void ClearChildren();

    void SetCenter(glm::vec2 pos);
    glm::vec2 GetCenter();
    void SetPosition(glm::vec2 pos);
    void AdjustPosition(glm::vec2 adj);
    void SetDimensions(glm::vec2 dims);

    void SetTexture(Graphics::TextureIndex);
    void SetColorMode(Graphics::ColorMode);
    void SetColor(glm::vec4);

    std::size_t size() const;

protected:
    bool Within(glm::vec2 click);
    MouseEvent TransformEvent(const MouseEvent&);

    Graphics::DrawInfo mDrawInfo;
    Graphics::PositionInfo mPositionInfo;
    std::vector<Widget*> mChildren;
    bool mActive;
};

}
