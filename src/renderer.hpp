#pragma once

#include "worldFactory.hpp"

#include <GL/glew.h>

#include <boost/range/adaptor/indexed.hpp>

#include <cmath>

namespace BAK {

class GLBuffers
{
public:
    GLBuffers()
    :
        mVertexBuffer{GenBufferGL()},
        mNormalBuffer{GenBufferGL()},
        mColorBuffer{GenBufferGL()},
        mTextureCoordBuffer{GenBufferGL()},
        mTextureBlendBuffer{GenBufferGL()},
        mElementBuffer{GenBufferGL()}
    {}

    ~GLBuffers()
    {
        // Any other destruction needed here?
        glDeleteBuffers(1, &mVertexBuffer);
        glDeleteBuffers(1, &mNormalBuffer);
        glDeleteBuffers(1, &mColorBuffer);
        glDeleteBuffers(1, &mTextureCoordBuffer);
        glDeleteBuffers(1, &mTextureBlendBuffer);
        glDeleteBuffers(1, &mElementBuffer);
    }
    
    static GLuint GenBufferGL()
    {
        GLuint buffer;
        glGenBuffers(1, &buffer);
        return buffer;
    }
    
    template <typename T>
    void LoadBufferDataGL(GLuint buffer, GLenum target, const std::vector<T>& data)
    {
        glBindBuffer(target, buffer);
        glBufferData(
            target,
            data.size() * sizeof(T),
            &data.front(),
            // This will need to change...
            GL_STATIC_DRAW);
    }

    void BindAttribArrayGL(
        unsigned location,
        unsigned elems,
        GLuint buffer)
    {
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        // FIXME: All these attributes should get stored somewhere
        glVertexAttribPointer(
            location,
            elems,
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );
    }

    void BindArraysGL()
    {
        BindAttribArrayGL(mVertexLoc, 3, mVertexBuffer);
        BindAttribArrayGL(mNormalLoc, 3, mNormalBuffer);
        BindAttribArrayGL(mColorLoc, 4, mColorBuffer);
        BindAttribArrayGL(mTextureCoordLoc, 3, mTextureCoordBuffer);
        BindAttribArrayGL(mTextureBlendLoc, 1, mTextureBlendBuffer);
    }
    
//private:

// Locations in the shader
    static constexpr unsigned mVertexLoc       = 0;
    static constexpr unsigned mNormalLoc       = 1;
    static constexpr unsigned mColorLoc        = 2;
    static constexpr unsigned mTextureCoordLoc = 3;
    static constexpr unsigned mTextureBlendLoc = 4;

    GLuint mVertexBuffer;
    GLuint mNormalBuffer;
    GLuint mColorBuffer;
    GLuint mTextureCoordBuffer;
    GLuint mTextureBlendBuffer;
    GLuint mElementBuffer;
};

class TextureBuffer
{
public:

    TextureBuffer()
    :
        mTextureBuffer{
            std::invoke([](){
                unsigned texture;
                glGenTextures(1, &texture);
                return texture;
            })
        }
    {}

    void LoadTexturesGL(const TextureStore& textures)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, mTextureBuffer);
        auto maxDim = textures.GetMaxDim();

        glTexStorage3D(
            GL_TEXTURE_2D_ARRAY,
            1, 
            GL_RGBA8,        // Internal format
            maxDim, maxDim, // width,height
            64              // Number of layers
        );
        
        for (const auto& tex : textures.GetTextures() | boost::adaptors::indexed())
        {
            std::vector<glm::vec4> paddedTex(
                maxDim * maxDim,
                glm::vec4{0, 0, 0, 0});

            // Chuck the image in the padded sized texture
            for (unsigned x = 0; x < tex.value().mWidth; x++)
                for (unsigned y = 0; y < tex.value().mHeight; y++)
                    paddedTex[x + y * maxDim] = tex.value().mTexture[x + y * tex.value().mWidth];

            glTexSubImage3D(
                GL_TEXTURE_2D_ARRAY,
                0,                 // Mipmap number
                0, 0, tex.index(), // xoffset, yoffset, zoffset
                maxDim, maxDim, 1, // width, height, depth
                GL_RGBA,           // format
                GL_FLOAT,          // type
                paddedTex.data()); // pointer to data
        }
        
        //glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);   
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

//private:
    GLuint mTextureBuffer;
};

}
