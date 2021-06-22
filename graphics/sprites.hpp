#pragma once

#include "graphics/plane.hpp"
#include "graphics/renderer.hpp"
#include "graphics/texture.hpp"

#include "com/logger.hpp"

#include <GL/glew.h>

namespace Graphics {

class Sprites
{
    Sprites()
        mBuffers{},
        mTextureBuffer{},
        mObjects{}
    {
    }

    void BindGL()
    {
        mVertexArray.BindGL();
        mTextureBuffer.BindGL();
    }

    void UnbindGL()
    {
        mVertexArray.UnbindGL();
        mTextureBuffer.UnbindGL();
    }

    void LoadGL(const TextureStore& textures)
    {
        mTextureBuffer.LoadTexturesGL(
            textures.GetTextures(),
            textures.GetMaxDim());

        for (unsigned i = 0; i < textures.GetTextures().size(); i++)
        {
            const auto& tex = textures.GetTexture(i);
            objStore.AddObject(
                Graphics::Quad{
                    static_cast<double>(tex.GetWidth()),
                    static_cast<double>(tex.GetHeight()),
                    static_cast<double>(textures.GetMaxDim()),
                    i});
        }

        mVertexArray.BindGL();

        buffers.AddBuffer("vertex", 0, 3);
        buffers.AddBuffer("textureCoord", 1, 3);

        buffers.LoadBufferDataGL("vertex", GL_ARRAY_BUFFER, objStore.mVertices);
        buffers.LoadBufferDataGL("textureCoord", GL_ARRAY_BUFFER, objStore.mTextureCoords);
        buffers.LoadBufferDataGL(buffers.mElementBuffer, GL_ELEMENT_ARRAY_BUFFER, objStore.mIndices);
        buffers.BindArraysGL();
        
        UnbindGL();
    }
    
    VertexArrayObject mVertexArray;
    GLBuffers mBuffers;
    TextureBuffer mTextureBuffer;
    QuadStorage mObjects;
};

}
