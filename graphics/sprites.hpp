#pragma once

#include "graphics/plane.hpp"
#include "graphics/renderer.hpp"
#include "graphics/texture.hpp"

#include "com/logger.hpp"

#include <GL/glew.h>

namespace Graphics {

class Sprites
{
public:
    Sprites()
    :
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

    void LoadTexturesGL(const TextureStore& textures)
    {
        mTextureBuffer.LoadTexturesGL(
            textures.GetTextures(),
            textures.GetMaxDim());

        for (unsigned i = 0; i < textures.GetTextures().size(); i++)
        {
            const auto& tex = textures.GetTexture(i);
            mObjects.AddObject(
                Quad{
                    static_cast<double>(tex.GetWidth()),
                    static_cast<double>(tex.GetHeight()),
                    static_cast<double>(textures.GetMaxDim()),
                    i});
        }

        mVertexArray.BindGL();

        mBuffers.AddBuffer("vertex", 0, 3);
        mBuffers.AddBuffer("textureCoord", 1, 3);

        mBuffers.LoadBufferDataGL("vertex", GL_ARRAY_BUFFER, mObjects.mVertices);
        mBuffers.LoadBufferDataGL("textureCoord", GL_ARRAY_BUFFER, mObjects.mTextureCoords);
        mBuffers.LoadBufferDataGL(mBuffers.mElementBuffer, GL_ELEMENT_ARRAY_BUFFER, mObjects.mIndices);
        mBuffers.BindArraysGL();
        
        UnbindGL();
    }
    
    VertexArrayObject mVertexArray;
    GLBuffers mBuffers;
    TextureBuffer mTextureBuffer;
    QuadStorage mObjects;
};

}
