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
    Renderer()
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
        mDepthFB{},
        mDepthBuffer{GL_TEXTURE_2D}
    {
        mDepthBuffer.MakeDepthBuffer(8096, 8096);
        mDepthFB.AttachDepthTexture(mDepthBuffer);
    }

    template <typename TextureStoreT>
    void LoadData(
        const MeshObjectStorage& objectStore,
        const TextureStoreT& textureStore)
    {
        // FIXME Issue 48: Need to do destruct and restruct all the buffers
        // when we load new data...
        mVertexArrayObject.BindGL();

        mGLBuffers.AddBuffer("vertex", 0, 3);
        mGLBuffers.AddBuffer("normal", 1, 3);
        mGLBuffers.AddBuffer("color", 2, 4);
        mGLBuffers.AddBuffer("textureCoord", 3, 3);
        mGLBuffers.AddBuffer("textureBlend", 4, 1);

        mGLBuffers.LoadBufferDataGL("vertex", GL_ARRAY_BUFFER, objectStore.mVertices);
        mGLBuffers.LoadBufferDataGL("normal", GL_ARRAY_BUFFER, objectStore.mNormals);
        mGLBuffers.LoadBufferDataGL("color", GL_ARRAY_BUFFER, objectStore.mColors);
        mGLBuffers.LoadBufferDataGL("textureCoord", GL_ARRAY_BUFFER, objectStore.mTextureCoords);
        mGLBuffers.LoadBufferDataGL("textureBlend", GL_ARRAY_BUFFER, objectStore.mTextureBlends);
        mGLBuffers.LoadBufferDataGL(mGLBuffers.mElementBuffer, GL_ELEMENT_ARRAY_BUFFER, objectStore.mIndices);

        mGLBuffers.BindArraysGL();

        mTextureBuffer.LoadTexturesGL(
            textureStore.GetTextures(),
            textureStore.GetMaxDim());
    }

    template <typename Renderables, typename Camera>
    void DrawShadow(
        const Renderables& renderables,
        const Light& light,
        const Camera& lightCamera,
        const Camera& camera)
    {
        mVertexArrayObject.BindGL();
        glActiveTexture(GL_TEXTURE0);
        mTextureBuffer.BindGL();

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mDepthBuffer.GetId());

        auto& shader = mModelShader;
        shader.UseProgramGL();

        shader.SetUniform(shader.GetUniformLocation("texture0"), 0);
        shader.SetUniform(shader.GetUniformLocation("shadowMap"), 1);

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

        auto viewMatrix = camera.GetViewMatrix();
        glm::mat4 MVP;

        for (const auto& item : renderables)
        {
            if (glm::distance(camera.GetPosition(), item.GetLocation()) > 128000.0) continue;
            const auto [offset, length] = item.GetObject();
            auto modelMatrix = item.GetModelMatrix();

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

    template <typename Renderables, typename Camera>
    void Draw(
        const Renderables& renderables,
        const Camera& lightCamera)
    {
        mVertexArrayObject.BindGL();

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
    FrameBuffer mDepthFB;
    TextureBuffer mDepthBuffer;
};

}
