#pragma once

#include "graphics/plane.hpp"
#include "graphics/renderer.hpp"
#include "graphics/texture.hpp"
#include "graphics/types.hpp"

#include "com/logger.hpp"

#include <GL/glew.h>

namespace Graphics {

class Sprites
{
public:
    Sprites()
    :
        mVertexArray{},
        mBuffers{},
        mTextureBuffer{},
        mObjects{}
    {
    }

    Sprites(const Sprites& other) = delete;
    Sprites& operator=(const Sprites& other) = delete;

    Sprites(Sprites&& other)
    :
        mVertexArray{std::move(other.mVertexArray)},
        mBuffers{std::move(other.mBuffers)},
        mTextureBuffer{std::move(other.mTextureBuffer)},
        mObjects{other.mObjects}
    {
    }

    Sprites& operator=(Sprites&& other)
    {
        this->mVertexArray = std::move(other.mVertexArray);
        this->mBuffers = std::move(other.mBuffers);
        this->mTextureBuffer = std::move(other.mTextureBuffer);
        this->mObjects = other.mObjects;
        return *this;
    }

    void BindGL() const
    {
        mVertexArray.BindGL();
        mTextureBuffer.BindGL();
    }

    void UnbindGL() const
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

    std::size_t size()
    {
        return mObjects.size();
    }

    auto Get(unsigned i) const
    {
        return mObjects.GetObject(i);
    }

private:
    VertexArrayObject mVertexArray;
    GLBuffers mBuffers;
    TextureBuffer mTextureBuffer;
    QuadStorage mObjects;
};

class SpriteManager
{
public:
    SpriteManager()
    :
        mSpriteSheets{},
        mNextSpriteSheet{1},
        mActiveSpriteSheet{0}
    {
    }

    auto AddSpriteSheet()
    {
        const auto spriteSheetIndex = NextSpriteSheet();
        auto [it, emplaced] = mSpriteSheets.emplace(
            spriteSheetIndex,
            Sprites{});
        assert(emplaced);
        assert(it != mSpriteSheets.end());
        return it;
    }

    void ActivateSpriteSheet(SpriteSheetIndex spriteSheet)
    {
        if (spriteSheet != mActiveSpriteSheet)
            GetSpriteSheet(spriteSheet).BindGL();
    }

    Sprites& GetSpriteSheet(SpriteSheetIndex spriteSheet)
    {
        assert(mSpriteSheets.contains(spriteSheet));
        return mSpriteSheets[spriteSheet];
    }

private:
    SpriteSheetIndex NextSpriteSheet()
    {
        return mNextSpriteSheet++;
    }

    std::unordered_map<
        SpriteSheetIndex,
        Sprites> mSpriteSheets;

    SpriteSheetIndex mNextSpriteSheet;
    SpriteSheetIndex mActiveSpriteSheet;
};

}
