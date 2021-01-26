#pragma once

#include "logger.hpp"

#include "worldFactory.hpp"

#include <GL/glew.h>

#include <boost/range/adaptor/indexed.hpp>

#include <cmath>

namespace BAK {

enum class BindPoint
{
    ArrayBuffer,
    ElementArrayBuffer
};

GLenum ToGlEnum(BindPoint p)
{
    switch (p)
    {
    case BindPoint::ArrayBuffer: return GL_ARRAY_BUFFER;
    case BindPoint::ElementArrayBuffer: return GL_ELEMENT_ARRAY_BUFFER;
    default: return GL_ARRAY_BUFFER;
    }
}

class GLBuffers
{
public:
    GLBuffers()
    :
        mBuffers{},
        mElementBuffer{GenBufferGL()},
        mActive{true}
    {
    }

    GLBuffers(GLBuffers&& other)
    {
        (*this) = std::move(other);
    }

    GLBuffers& operator=(GLBuffers&& other)
    {
        for (const auto& [name, buffer] : mBuffers)
            mBuffers.emplace(name, buffer);

        other.mActive = false;

        return *this;
    }

    GLBuffers(const GLBuffers&) = delete;
    GLBuffers& operator=(const GLBuffers&) = delete;

    ~GLBuffers()
    {
        if (mActive)
        {
            Logging::LogDebug("GLBuffers") << "Deleting GL buffers" << std::endl;
            for (const auto& [name, buffer] : mBuffers)
            {
                Logging::LogDebug("GLBuffers") << "    Deleting buffer: " 
                    << name << std::endl;
                glDeleteBuffers(1, &buffer.mBuffer);
            }
        }
    }

    void AddBuffer(
        const std::string& name,
        unsigned location,
        unsigned elems)
    {
        mBuffers.emplace(
            name,
            GLBuffer{location, elems, BindPoint::ArrayBuffer, GenBufferGL()});
        BindAttribArrayGL(location, elems, mBuffers[name].mBuffer);
    }
    
    static GLuint GenBufferGL()
    {
        GLuint buffer;
        glGenBuffers(1, &buffer);
        return buffer;
    }
    
    template <typename T>
    void LoadBufferDataGL(
        const std::string& buffer,
        GLenum target,
        const std::vector<T>& data)
    {
        LoadBufferDataGL(
            mBuffers[buffer].mBuffer,
            target,
            data);
    }

    template <typename T>
    void LoadBufferDataGL(
        GLuint buffer,
        GLenum target,
        const std::vector<T>& data)
    {
        glBindBuffer(target, buffer);
        glBufferData(
            target,
            data.size() * sizeof(T),
            &data.front(),
            // This will need to change...
            GL_DYNAMIC_DRAW);
            //GL_STATIC_DRAW);
    }

    template <typename T>
    void ModifyBufferDataGL(
        const std::string& name,
        GLenum target,
        unsigned offset,
        const std::vector<T>& data)
    {
        glBindBuffer(target, mBuffers[name].mBuffer);
        glBufferSubData(
            target,
            offset,
            data.size() * sizeof(T),
            &data.front());
    }

    void BindAttribArrayGL(
        unsigned location,
        unsigned elems,
        GLuint buffer)
    {
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
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
        for (const auto& [name, buffer] : mBuffers)
            BindAttribArrayGL(buffer.mLocation, buffer.mElems, buffer.mBuffer);
    }
    
//private:
    struct GLBuffer
    {
        // Location in the shader
        unsigned mLocation;
        // Elements per object (e.g. color = 4 floats)
        unsigned mElems;
        // BindPoint (ARRAY, ELEMENT_ARRAY, etc)
        BindPoint mBindPoint;
        // GL assigned buffer id
        GLuint mBuffer;
    };

    std::unordered_map<std::string, GLBuffer> mBuffers;

    GLuint mElementBuffer;

    // disable when moving from
    bool mActive;
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
            5, 
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
        
        // Doesn't actually look very good with mipmaps...
        //glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);   
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

//private:
    GLuint mTextureBuffer;
};

}
