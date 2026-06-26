#pragma once

#include "graphics/meshObject.hpp"
#include "graphics/opengl.hpp"
#include "graphics/framebuffer.hpp"
#include "graphics/renderData.hpp"
#include "graphics/shaderProgram.hpp"

#include <cstdint>
#include <optional>

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

struct PickShaderUniforms
{
    GLuint mTexture0;
    GLuint mMVP;
    GLuint mEntityId;
    GLuint mM;
    GLuint mCameraPosition_worldSpace;
};

struct WorldShaderUniforms
{
    GLuint mTexture0;
    GLuint mShadowMap;
    GLuint mFogStrength;
    GLuint mFogColor;
    GLuint mLightDirection;
    GLuint mLightAmbientColor;
    GLuint mLightDiffuseColor;
    GLuint mLightSpecularColor;
    GLuint mLightSpaceMatrix;
    GLuint mCameraPosition_worldspace;
    GLuint mMVP;
    GLuint mM;
    GLuint mV;
};

struct DepthMapShaderUniforms
{
    GLuint mTexture0;
    GLuint mLightSpaceMatrix;
    GLuint mM;
};

class Renderer
{
    static constexpr auto sClickDistance = 16000;
public:
    Renderer(
        float screenWidth,
        float screenHeight,
        unsigned depthMapWidth,
        unsigned depthMapHeight,
        int drawDistance = 128000)
    :
        mModelShader{std::invoke([]{
            auto shader = ShaderProgram{
                "directional.vert.glsl",
                "directional.frag.glsl"};
            return shader.Compile();
        })},
        mModelShaderUniforms{
            mModelShader.GetUniformLocation("texture0"),
            mModelShader.GetUniformLocation("shadowMap"),
            mModelShader.GetUniformLocation("fogStrength"),
            mModelShader.GetUniformLocation("fogColor"),
            mModelShader.GetUniformLocation("light.mDirection"),
            mModelShader.GetUniformLocation("light.mAmbientColor"),
            mModelShader.GetUniformLocation("light.mDiffuseColor"),
            mModelShader.GetUniformLocation("light.mSpecularColor"),
            mModelShader.GetUniformLocation("lightSpaceMatrix"),
            mModelShader.GetUniformLocation("cameraPosition_worldspace"),
            mModelShader.GetUniformLocation("MVP"),
            mModelShader.GetUniformLocation("M"),
            mModelShader.GetUniformLocation("V")
        },
        mSpriteShader{std::invoke([]{
            auto shader = ShaderProgram{
                "sprite.vert.glsl",
                "sprite.frag.glsl"};
            return shader.Compile();
        })},
        mSpriteShaderUniforms{
            mSpriteShader.GetUniformLocation("texture0"),
            mSpriteShader.GetUniformLocation("shadowMap"),
            mSpriteShader.GetUniformLocation("fogStrength"),
            mSpriteShader.GetUniformLocation("fogColor"),
            mSpriteShader.GetUniformLocation("light.mDirection"),
            mSpriteShader.GetUniformLocation("light.mAmbientColor"),
            mSpriteShader.GetUniformLocation("light.mDiffuseColor"),
            mSpriteShader.GetUniformLocation("light.mSpecularColor"),
            mSpriteShader.GetUniformLocation("lightSpaceMatrix"),
            mSpriteShader.GetUniformLocation("cameraPosition_worldspace"),
            mSpriteShader.GetUniformLocation("MVP"),
            mSpriteShader.GetUniformLocation("M"),
            mSpriteShader.GetUniformLocation("V")
        },
        mPickShader{std::invoke([]{
            auto shader = ShaderProgram{
                "pick.vert.glsl",
                "pick.frag.glsl"};
            return shader.Compile();
        })},
        mPickShaderUniforms{
            mPickShader.GetUniformLocation("texture0"),
            mPickShader.GetUniformLocation("MVP"),
            mPickShader.GetUniformLocation("entityId"),
            0,
            0
        },
        mPickSpriteShader{std::invoke([]{
            auto shader = ShaderProgram{
                "pick.sprite.vert.glsl",
                "pick.sprite.frag.glsl"};
            return shader.Compile();
        })},
        mPickSpriteShaderUniforms{
            mPickSpriteShader.GetUniformLocation("texture0"),
            mPickSpriteShader.GetUniformLocation("MVP"),
            mPickSpriteShader.GetUniformLocation("entityId"),
            mPickSpriteShader.GetUniformLocation("M"),
            mPickSpriteShader.GetUniformLocation("cameraPosition_worldspace")
        },
        mShadowMapShader{std::invoke([]{
            auto shader = ShaderProgram{
                "shadowMap.vert.glsl",
                "shadowMap.frag.glsl"
            };
            return shader.Compile();
        })},
        mShadowMapShaderUniforms{
            mShadowMapShader.GetUniformLocation("texture0"),
            mShadowMapShader.GetUniformLocation("lightSpaceMatrix"),
            mShadowMapShader.GetUniformLocation("M")
        },
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
        mDrawDistance{drawDistance},
        mDepthMapDims{depthMapWidth, depthMapHeight},
        mDepthFB{},
        mDepthBuffer{GL_TEXTURE_2D},
        mScreenDims{screenWidth, screenHeight}
    {
        mPickTexture.MakePickBuffer(screenWidth, screenHeight);
        mPickDepth.MakeDepthBuffer(screenWidth, screenHeight);
        mPickFB.AttachTexture(mPickTexture);
        mPickFB.AttachDepthTexture(mPickDepth, false);

        mHoverPBO.Allocate<glm::vec4>(GL_DYNAMIC_READ);

        mDepthBuffer.MakeDepthBuffer(
            mDepthMapDims.x, 
            mDepthMapDims.y);
        mDepthFB.AttachDepthTexture(mDepthBuffer, true);
    }

    template <typename Renderables, typename DynamicRenderables, typename Camera>
    void DrawForPicking(
        const RenderData& renderData,
        const Renderables& renderables,
        const Renderables& sprites,
        const DynamicRenderables& dynamicRenderables,
        const Camera& camera)
    {
        renderData.Bind(GL_TEXTURE0);

        mPickFB.BindGL();
        glViewport(0, 0, mScreenDims.x, mScreenDims.y);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto* shader = &mPickShader;
        auto* uniforms = &mPickShaderUniforms;
        shader->UseProgramGL();

        shader->SetUniform(uniforms->mTexture0, 0);

        const auto& viewMatrix = camera.GetViewMatrix();
        glm::mat4 MVP;

        const auto RenderItem = [&](const auto& item, bool isSprite)
        {
            if (glm::distance(camera.GetPosition(), item.GetLocation()) > sClickDistance) return;

            const auto [offset, length] = item.GetObject();
            const auto& modelMatrix = item.GetModelMatrix();

            MVP = camera.GetProjectionMatrix() * viewMatrix * modelMatrix;

            shader->SetUniform(uniforms->mMVP, MVP);
            shader->SetUniform(uniforms->mEntityId, item.GetId().mValue);
            if (isSprite)
            {
                shader->SetUniform(uniforms->mM, modelMatrix);
                shader->SetUniform(uniforms->mCameraPosition_worldSpace, camera.GetPosition());
            }

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
            RenderItem(item, false);
        }

        shader = &mPickSpriteShader;
        uniforms = &mPickSpriteShaderUniforms;
        shader->UseProgramGL();

        shader->SetUniform(uniforms->mEntityId, 0u);

        for (const auto& item : sprites)
        {
            RenderItem(item, true);
        }

        for (const auto& item : dynamicRenderables)
        {
            item.GetRenderData()->Bind(GL_TEXTURE0);
            RenderItem(item, true);
        }

        mPickFB.UnbindGL();
    }

    template <typename Renderables, typename Camera>
    void DrawWithShadow(
        const RenderData& renderData,
        const Renderables& renderables,
        const Light& light,
        const Camera& lightCamera,
        const Camera& camera,
        bool isSprite)
    {
        renderData.Bind(GL_TEXTURE0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mDepthBuffer.GetId());

        auto& shader = isSprite ? mSpriteShader : mModelShader;
        auto& uniforms = isSprite ? mSpriteShaderUniforms : mModelShaderUniforms;
        shader.UseProgramGL();

        shader.SetUniform(uniforms.mTexture0, 0);
        shader.SetUniform(uniforms.mShadowMap, 1);

        shader.SetUniform(uniforms.mFogStrength, Float{light.mFogStrength});
        shader.SetUniform(uniforms.mFogColor, light.mFogColor);

        shader.SetUniform(uniforms.mLightDirection, light.mDirection);
        shader.SetUniform(uniforms.mLightAmbientColor, light.mAmbientColor);
        shader.SetUniform(uniforms.mLightDiffuseColor, light.mDiffuseColor);
        shader.SetUniform(uniforms.mLightSpecularColor, light.mSpecularColor);

        shader.SetUniform(
            uniforms.mLightSpaceMatrix,
            lightCamera.GetProjectionMatrix() * lightCamera.GetViewMatrix());

        shader.SetUniform(uniforms.mCameraPosition_worldspace, camera.GetNormalisedPosition());

        const auto mvpMatrixId = uniforms.mMVP;
        const auto modelMatrixId = uniforms.mM;
        const auto viewMatrixId = uniforms.mV;

        const auto& viewMatrix = camera.GetViewMatrix();
        glm::mat4 MVP;

        for (const auto& item : renderables)
        {
            if (glm::distance(camera.GetPosition(), item.GetLocation()) > mDrawDistance) continue;
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
        return DecodeEntityId(data);
    }

    std::optional<std::uint32_t> GetHoveredEntity()
    {
        CheckHover();
        return mHoveredEntity;
    }

    void StartPickReadback(glm::vec2 mousePos)
    {
        mPickFB.BindGL();
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        mHoverPBO.BindGL();
        glReadPixels(
            mousePos.x, mScreenDims.y - mousePos.y,
            1, 1, GL_RGBA, GL_FLOAT, nullptr);
        mHoverPBO.UnbindGL();
        mPickFB.UnbindGL();

        mPickReadbackIssued = true;
    }

    void BeginDepthMapDraw()
    {
        mDepthFB.BindGL();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, mDepthMapDims.x, mDepthMapDims.y);
    }

    void EndDepthMapDraw()
    {
        mDepthFB.UnbindGL();
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

        shader.SetUniform(mShadowMapShaderUniforms.mTexture0, 0);
        const auto lightSpaceMatrixId = mShadowMapShaderUniforms.mLightSpaceMatrix;
        shader.SetUniform(
            lightSpaceMatrixId,
            lightCamera.GetProjectionMatrix() * lightCamera.GetViewMatrix());

        const auto modelMatrixId = mShadowMapShaderUniforms.mM;

        for (const auto& item : renderables)
        {
            if (glm::distance(lightCamera.GetPosition(), item.GetLocation()) > mDrawDistance) continue;
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

private:
    static unsigned DecodeEntityId(const glm::vec4& data)
    {
        return static_cast<unsigned>(data.r)
            | (static_cast<unsigned>(data.g) << 8)
            | (static_cast<unsigned>(data.b) << 16)
            | (static_cast<unsigned>(data.a) << 24);
    }

    void CheckHover()
    {
        if (!mPickReadbackIssued)
        {
            mHoveredEntity.reset();
            return;
        }

        mHoverPBO.BindGL();

        if (void* data = mHoverPBO.Map(GL_READ_ONLY))
        {
            glm::vec4 result;
            std::memcpy(&result, data, sizeof(result));
            mHoverPBO.Unmap();
            mHoveredEntity = DecodeEntityId(result);
        }
        else
        {
            mHoveredEntity.reset();
        }

        mHoverPBO.UnbindGL();
    }

    ShaderProgramHandle mModelShader;
    WorldShaderUniforms mModelShaderUniforms;
    ShaderProgramHandle mSpriteShader;
    WorldShaderUniforms mSpriteShaderUniforms;
    ShaderProgramHandle mPickShader;
    PickShaderUniforms mPickShaderUniforms;
    ShaderProgramHandle mPickSpriteShader;
    PickShaderUniforms mPickSpriteShaderUniforms;
    ShaderProgramHandle mShadowMapShader;
    DepthMapShaderUniforms mShadowMapShaderUniforms;
    ShaderProgramHandle mNormalShader;

    FrameBuffer mPickFB;
    TextureBuffer mPickTexture;
    TextureBuffer mPickDepth;
    PixelPackBuffer mHoverPBO{};
    std::optional<std::uint32_t> mHoveredEntity{};
    bool mPickReadbackIssued{false};
    int mDrawDistance;
    glm::uvec2 mDepthMapDims;
    FrameBuffer mDepthFB;
    TextureBuffer mDepthBuffer;
    glm::vec2 mScreenDims;
};

}
