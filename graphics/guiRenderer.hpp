#pragma once

#include "graphics/shaderProgram.hpp"
#include "graphics/texture.hpp"
#include "graphics/types.hpp"

#include "com/logger.hpp"

#include <GL/glew.h>

#include <cmath>

namespace Graphics {

enum class ColorMode
{
    // Use the color from the given texture
    Texture = 0,
    // Use a solid color
    SolidColor = 1,
    // Tint the texture this color (respects texture alpha)
    TintColor = 2 
};

// Tightly coupled with the GUI shader

class GuiCamera
{
public:

    void UpdateModelViewMatrix(const glm::mat4& modelMatrix)
    {
        mModelMatrix = modelMatrix;
        mMVP = mViewMatrix * mScaleMatrix * mModelMatrix;

        ShaderProgramHandle::SetUniform(mMvpMatrixId  , mMVP);
        ShaderProgramHandle::SetUniform(mModelMatrixId, mModelMatrix);
        ShaderProgramHandle::SetUniform(mViewMatrixId , mViewMatrix);
    }

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
        float scale)
    :
        mShader{ShaderProgram{vertexShader, fragmentShader}
            .Compile()},
        mCamera{
            glm::scale(glm::mat4{1}, glm::vec3{scale, scale, 0}),
            glm::ortho(
                0.0f,
                width,
                height,
                0.0f,
                -1.0f,
                1.0f), // view
            glm::mat4{1.0f}, // model
            glm::mat4{1.0f}, // MVP
            mShader.GetUniformLocation("MVP"),
            mShader.GetUniformLocation("M"),
            mShader.GetUniformLocation("V")
        },
        mBlockColorId{mShader.GetUniformLocation("blockColor")},
        mColorModeId{mShader.GetUniformLocation("colorMode")}
    {}

    void Draw(
        const glm::mat4& modelMatrix,
        ColorMode colorMode,
        const glm::vec4& blockColor,
        TextureIndex texture,
        std::tuple<unsigned, unsigned> object)
    {
        mCamera.UpdateModelViewMatrix(modelMatrix);

        ShaderProgramHandle::SetUniform(mColorModeId, static_cast<unsigned>(colorMode));
        ShaderProgramHandle::SetUniform(mBlockColorId, blockColor);
        
        const auto [offset, length] = object;

        glDrawElementsBaseVertex(
            GL_TRIANGLES,
            length,
            GL_UNSIGNED_INT,
            (void*) (offset * sizeof(GLuint)),
            offset
        );
    }

private:
    ShaderProgramHandle mShader;

    GuiCamera mCamera;

    // These are straight from the shader...
    GLuint mBlockColorId;
    GLuint mColorModeId;
};

}
