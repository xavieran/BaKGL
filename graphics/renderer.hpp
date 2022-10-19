#pragma once

#include "graphics/meshObject.hpp"
#include "graphics/opengl.hpp"
#include "graphics/framebuffer.hpp"
#include "graphics/shaderProgram.hpp"

namespace Graphics {

struct Light
{
    glm::vec3 mDirection;
    glm::vec3 mAmbientColor;
    glm::vec3 mDiffuseColor;
    glm::vec3 mSpecularColor;
};

class Renderer
{
public:
    Renderer(
        unsigned depthMapWidth,
        unsigned depthMapHeight)
    :
        mModelShader{std::invoke([]{
            auto shader = ShaderProgram{
                "directional.vert.glsl",
                "directional.frag.glsl"};
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
        mVertexArrayObject{},
        mGLBuffers{},
        mTextureBuffer{GL_TEXTURE_2D_ARRAY},
        mDepthMapDims{depthMapWidth, depthMapHeight},
        mDepthFB1{},
        mDepthFB2{},
        mDepthBuffer1{GL_TEXTURE_2D},
        mDepthBuffer2{GL_TEXTURE_2D},
        mUseDepthBuffer1{false}
    {
        mDepthBuffer1.MakeDepthBuffer(
            mDepthMapDims.x, 
            mDepthMapDims.y);
        mDepthFB1.AttachDepthTexture(mDepthBuffer1);

        mDepthBuffer2.MakeDepthBuffer(
            mDepthMapDims.x, 
            mDepthMapDims.y);
        mDepthFB2.AttachDepthTexture(mDepthBuffer2);
    }

    template <typename TextureStoreT>
    void LoadData(
        const MeshObjectStorage& objectStore,
        const TextureStoreT& textureStore)
    {
        // FIXME Issue 48: Need to do destruct and restruct all the buffers
        // when we load new data...
        mVertexArrayObject.BindGL();

        mGLBuffers.AddStaticArrayBuffer<glm::vec3>("vertex", GLLocation{0});
        mGLBuffers.AddStaticArrayBuffer<glm::vec3>("normal", GLLocation{1});
        mGLBuffers.AddStaticArrayBuffer<glm::vec4>("color", GLLocation{2});
        mGLBuffers.AddStaticArrayBuffer<glm::vec3>("textureCoord", GLLocation{3});
        mGLBuffers.AddStaticArrayBuffer<glm::vec1>("textureBlend", GLLocation{4});
        mGLBuffers.AddElementBuffer("elements");

        mGLBuffers.LoadBufferDataGL("vertex", objectStore.mVertices);
        mGLBuffers.LoadBufferDataGL("normal", objectStore.mNormals);
        mGLBuffers.LoadBufferDataGL("color", objectStore.mColors);
        mGLBuffers.LoadBufferDataGL("textureCoord", objectStore.mTextureCoords);
        mGLBuffers.LoadBufferDataGL("textureBlend", objectStore.mTextureBlends);
        mGLBuffers.LoadBufferDataGL("elements", objectStore.mIndices);

        mGLBuffers.BindArraysGL();

        mTextureBuffer.LoadTexturesGL(
            textureStore.GetTextures(),
            textureStore.GetMaxDim());
    }

    template <typename Renderables, typename Camera>
    void DrawWithShadow(
        const Renderables& renderables,
        const Light& light,
        const Camera& lightCamera,
        const Camera& camera)
    {
        mVertexArrayObject.BindGL();
        glActiveTexture(GL_TEXTURE0);
        mTextureBuffer.BindGL();

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,
            mUseDepthBuffer1
            ? mDepthBuffer1.GetId()
            : mDepthBuffer2.GetId());

        auto& shader = mModelShader;
        shader.UseProgramGL();

        shader.SetUniform(shader.GetUniformLocation("texture0"), 0);
        shader.SetUniform(shader.GetUniformLocation("shadowMap"), 1);

        shader.SetUniform(shader.GetUniformLocation("fogStrength"), Float{0.0005f});
        shader.SetUniform(shader.GetUniformLocation("fogColor"), glm::vec3{.15, .31, .36});

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
            if (glm::distance(camera.GetPosition(), item.GetLocation()) > 128000.0) continue;
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
        const Renderables& renderables,
        const Camera& lightCamera)
    {
        mVertexArrayObject.BindGL();

        auto& shader = mShadowMapShader;

        shader.UseProgramGL();

        // Required so we get correct depth for sprites with alpha
        glActiveTexture(GL_TEXTURE0);
        mTextureBuffer.BindGL();

        shader.SetUniform(shader.GetUniformLocation("texture0"), 0);
        const auto lightSpaceMatrixId = shader.GetUniformLocation("lightSpaceMatrix");
        shader.SetUniform(
            lightSpaceMatrixId,
            lightCamera.GetProjectionMatrix() * lightCamera.GetViewMatrix());

        const auto modelMatrixId = shader.GetUniformLocation("M");

        for (const auto& item : renderables)
        {
            if (glm::distance(lightCamera.GetPosition(), item.GetLocation()) > 128000.0) continue;
            const auto [offset, length] = item.GetObject();
            auto modelMatrix = item.GetModelMatrix();

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
    ShaderProgramHandle mShadowMapShader;
    ShaderProgramHandle mNormalShader;
    VertexArrayObject mVertexArrayObject;
    GLBuffers mGLBuffers;
    TextureBuffer mTextureBuffer;
    glm::uvec2 mDepthMapDims;
    FrameBuffer mDepthFB1;
    FrameBuffer mDepthFB2;
    TextureBuffer mDepthBuffer1;
    TextureBuffer mDepthBuffer2;
    bool mUseDepthBuffer1;
};

}
