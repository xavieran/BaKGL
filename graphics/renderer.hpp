#pragma once

#include "graphics/meshObject.hpp"
#include "graphics/opengl.hpp"
#include "graphics/framebuffer.hpp"
#include "graphics/renderData.hpp"
#include "graphics/shaderProgram.hpp"

namespace Graphics {

struct Light
{
    glm::vec3 mDirection;
    glm::vec3 mAmbientColor;
    glm::vec3 mDiffuseColor;
    glm::vec3 mSpecularColor;
    float mFogStrength;
    glm::vec3 mFogColor;
};

class Renderer
{
    static constexpr auto sDrawDistance = 128000;
    static constexpr auto sClickDistance = 16000;
public:
    Renderer(
        float screenWidth,
        float screenHeight,
        unsigned depthMapWidth,
        unsigned depthMapHeight)
    :
        mModelShader{std::invoke([]{
            auto shader = ShaderProgram{
                "directional.vert.glsl",
                "directional.frag.glsl"};
            return shader.Compile();
        })},
        mPickShader{std::invoke([]{
            auto shader = ShaderProgram{
                "pick.vert.glsl",
                "pick.frag.glsl"};
            return shader.Compile();
        })},
        mShadowMapShader{std::invoke([]{
            auto shader = ShaderProgram{
                "shadowMap.vert.glsl",
                "shadowMap.frag.glsl"
            };
            return shader.Compile();
        })},
        mNormalShader{std::invoke([]{
            auto shader = ShaderProgram{
                "see_norm.vert.glsl",
                "see_norm.geom.glsl",
                "see_norm.frag.glsl"
            };
            return shader.Compile();
        })},
        mPickFB{},
        mPickTexture{GL_TEXTURE_2D},
        mPickDepth{GL_TEXTURE_2D},
        mDepthMapDims{depthMapWidth, depthMapHeight},
        mDepthFB1{},
        mDepthFB2{},
        mDepthBuffer1{GL_TEXTURE_2D},
        mDepthBuffer2{GL_TEXTURE_2D},
        mScreenDims{screenWidth, screenHeight},
        mUseDepthBuffer1{false}
    {
        mPickTexture.MakePickBuffer(screenWidth, screenHeight);
        mPickDepth.MakeDepthBuffer(screenWidth, screenHeight);
        mPickFB.AttachTexture(mPickTexture);
        mPickFB.AttachDepthTexture(mPickDepth, false);

        mDepthBuffer1.MakeDepthBuffer(
            mDepthMapDims.x, 
            mDepthMapDims.y);
        mDepthFB1.AttachDepthTexture(mDepthBuffer1, true);

        mDepthBuffer2.MakeDepthBuffer(
            mDepthMapDims.x, 
            mDepthMapDims.y);
        mDepthFB2.AttachDepthTexture(mDepthBuffer2, true);
    }

    template <typename Renderables, typename Camera>
    void DrawForPicking(
        const RenderData& renderData,
        const Renderables& renderables,
        const Renderables& sprites,
        const Camera& camera)
    {
        renderData.Bind(GL_TEXTURE0);

        mPickFB.BindGL();
        glViewport(0, 0, mScreenDims.x, mScreenDims.y);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto& shader = mPickShader;
        shader.UseProgramGL();

        shader.SetUniform(shader.GetUniformLocation("texture0"), 0);

        const auto mvpMatrixId = shader.GetUniformLocation("MVP");
        const auto entityIdId = shader.GetUniformLocation("entityId");

        const auto& viewMatrix = camera.GetViewMatrix();
        glm::mat4 MVP;

        const auto RenderItem = [&](const auto& item)
        {
            if (glm::distance(camera.GetPosition(), item.GetLocation()) > sClickDistance) return;

            const auto [offset, length] = item.GetObject();
            const auto& modelMatrix = item.GetModelMatrix();

            MVP = camera.GetProjectionMatrix() * viewMatrix * modelMatrix;

            shader.SetUniform(mvpMatrixId, MVP);
            shader.SetUniform(entityIdId, item.GetId().mValue);

            glDrawElementsBaseVertex(
                GL_TRIANGLES,
                length,
                GL_UNSIGNED_INT,
                (void*) (offset * sizeof(GLuint)),
                offset
            );
        };

        for (const auto& item : renderables)
        {
            RenderItem(item);
        }
        for (const auto& item : sprites)
        {
            RenderItem(item);
        }

        mPickFB.UnbindGL();
        glFinish();
    }

    template <typename Renderables, typename Camera>
    void DrawWithShadow(
        const RenderData& renderData,
        const Renderables& renderables,
        const Light& light,
        const Camera& lightCamera,
        const Camera& camera)
    {
        renderData.Bind(GL_TEXTURE0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,
            mUseDepthBuffer1
            ? mDepthBuffer1.GetId()
            : mDepthBuffer2.GetId());

        auto& shader = mModelShader;
        shader.UseProgramGL();

        shader.SetUniform(shader.GetUniformLocation("texture0"), 0);
        shader.SetUniform(shader.GetUniformLocation("shadowMap"), 1);

        shader.SetUniform(shader.GetUniformLocation("fogStrength"), Float{light.mFogStrength});
        shader.SetUniform(shader.GetUniformLocation("fogColor"), light.mFogColor);

        shader.SetUniform(shader.GetUniformLocation("light.mDirection"), light.mDirection);
        shader.SetUniform(shader.GetUniformLocation("light.mAmbientColor"), light.mAmbientColor);
        shader.SetUniform(shader.GetUniformLocation("light.mDiffuseColor"), light.mDiffuseColor);
        shader.SetUniform(shader.GetUniformLocation("light.mSpecularColor"), light.mSpecularColor);

        shader.SetUniform(
            shader.GetUniformLocation("lightSpaceMatrix"),
            lightCamera.GetProjectionMatrix() * lightCamera.GetViewMatrix());

        shader.SetUniform(shader.GetUniformLocation("cameraPosition_worldspace"), camera.GetNormalisedPosition());

        const auto mvpMatrixId = shader.GetUniformLocation("MVP");
        const auto modelMatrixId = shader.GetUniformLocation("M");
        const auto viewMatrixId = shader.GetUniformLocation("V");

        const auto& viewMatrix = camera.GetViewMatrix();
        glm::mat4 MVP;

        for (const auto& item : renderables)
        {
            if (glm::distance(camera.GetPosition(), item.GetLocation()) > sDrawDistance) continue;
            const auto [offset, length] = item.GetObject();
            const auto& modelMatrix = item.GetModelMatrix();

            MVP = camera.GetProjectionMatrix() * viewMatrix * modelMatrix;

            shader.SetUniform(mvpMatrixId, MVP);
            shader.SetUniform(modelMatrixId, modelMatrix);
            shader.SetUniform(viewMatrixId, viewMatrix);

            glDrawElementsBaseVertex(
                GL_TRIANGLES,
                length,
                GL_UNSIGNED_INT,
                (void*) (offset * sizeof(GLuint)),
                offset
            );
        }
    }
    unsigned GetClickedEntity(glm::vec2 click)
    {
        mPickFB.BindGL();
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glm::vec4 data{};
        auto newClick = click;
        newClick.y = mScreenDims.y - click.y;
        glReadPixels(newClick.x, newClick.y, 1, 1, GL_RGBA, GL_FLOAT, &data);
        mPickFB.UnbindGL();
        const auto entityIndex = static_cast<unsigned>(data.r)
            | (static_cast<unsigned>(data.g) << 8)
            | (static_cast<unsigned>(data.b) << 16)
            | (static_cast<unsigned>(data.a) << 24);
        return entityIndex;
    }

    void BeginDepthMapDraw()
    {
        if (mUseDepthBuffer1)
        {
            mDepthFB1.BindGL();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, mDepthMapDims.x, mDepthMapDims.y);
        }
        else
        {
            mDepthFB2.BindGL();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, mDepthMapDims.x, mDepthMapDims.y);
        }
    }

    void EndDepthMapDraw()
    {
        if (mUseDepthBuffer1)
        {
            mDepthFB1.UnbindGL();
        }
        else
        {
            mDepthFB2.UnbindGL();
        }

        mUseDepthBuffer1 = !mUseDepthBuffer1;
    }

    template <typename Renderables, typename Camera>
    void DrawDepthMap(
        const RenderData& renderData,
        const Renderables& renderables,
        const Camera& lightCamera)
    {
        renderData.Bind(GL_TEXTURE0);

        auto& shader = mShadowMapShader;

        shader.UseProgramGL();

        shader.SetUniform(shader.GetUniformLocation("texture0"), 0);
        const auto lightSpaceMatrixId = shader.GetUniformLocation("lightSpaceMatrix");
        shader.SetUniform(
            lightSpaceMatrixId,
            lightCamera.GetProjectionMatrix() * lightCamera.GetViewMatrix());

        const auto modelMatrixId = shader.GetUniformLocation("M");

        for (const auto& item : renderables)
        {
            if (glm::distance(lightCamera.GetPosition(), item.GetLocation()) > sDrawDistance) continue;
            const auto [offset, length] = item.GetObject();
            const auto& modelMatrix = item.GetModelMatrix();

            shader.SetUniform(modelMatrixId, modelMatrix);

            glDrawElementsBaseVertex(
                GL_TRIANGLES,
                length,
                GL_UNSIGNED_INT,
                (void*) (offset * sizeof(GLuint)),
                offset
            );
        }
    }

//private:
    ShaderProgramHandle mModelShader;
    ShaderProgramHandle mPickShader;
    ShaderProgramHandle mShadowMapShader;
    ShaderProgramHandle mNormalShader;

    FrameBuffer mPickFB;
    TextureBuffer mPickTexture;
    TextureBuffer mPickDepth;
    glm::uvec2 mDepthMapDims;
    FrameBuffer mDepthFB1;
    FrameBuffer mDepthFB2;
    TextureBuffer mDepthBuffer1;
    TextureBuffer mDepthBuffer2;
    glm::vec2 mScreenDims;
    bool mUseDepthBuffer1;
};

}
