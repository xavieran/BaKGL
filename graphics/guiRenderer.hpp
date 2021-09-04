#pragma once

#include "graphics/IGuiElement.hpp"
#include "graphics/shaderProgram.hpp"
#include "graphics/sprites.hpp"
#include "graphics/texture.hpp"
#include "graphics/types.hpp"

#include "com/logger.hpp"

#include <GL/glew.h>

#include <cmath>

namespace Graphics {

// Tightly coupled with the GUI shader

class GuiCamera
{
public:
    GuiCamera(
        float width,
        float height,
        float scale,
        const ShaderProgramHandle& shader);
    
    void CalculateMatrices();
    void UpdateModelViewMatrix(const glm::mat4& modelMatrix);

    void ScissorRegion(glm::vec2 topLeft, glm::vec2 dimensions);
    void DisableScissor();

private:
    float mWidth;
    float mHeight;
    float mScale;

    glm::mat4 mScaleMatrix;
    glm::mat4 mViewMatrix;
    glm::mat4 mModelMatrix;

    glm::mat4 mMVP;

    GLuint mMvpMatrixId;
    GLuint mModelMatrixId;
    GLuint mViewMatrixId;

};

class GuiRenderer
{
public:

    static constexpr auto vertexShader   = "gui.vert.glsl";
    static constexpr auto fragmentShader = "gui.frag.glsl";

    GuiRenderer(
        float width,
        float height,
        float scale,
        SpriteManager& spriteManager);

    void RenderGui(
        Graphics::IGuiElement* element);

private:
    void RenderGuiImpl(
        glm::vec2 translate,
        Graphics::IGuiElement* element);
    
    void Draw(
        const glm::mat4& modelMatrix,
        ColorMode colorMode,
        const glm::vec4& blockColor,
        TextureIndex texture,
        std::tuple<unsigned, unsigned> object);

    ShaderProgramHandle mShader;
    SpriteManager& mSpriteManager;

    glm::vec3 mDimensions;
    GuiCamera mCamera;

    // These are straight from the shader...
    GLuint mBlockColorId;
    GLuint mColorModeId;

    unsigned mRenderCalls;

    const Logging::Logger& mLogger;
};

}
