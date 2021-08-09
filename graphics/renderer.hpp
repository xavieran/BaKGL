#pragma once

#include "graphics/texture.hpp"

#include "com/logger.hpp"

#include <GL/glew.h>

#include <cmath>

namespace Graphics {

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

class VertexArrayObject
{
public:
    VertexArrayObject()
    :
        mVertexArrayId{GenVertexArrayGL()},
        mActive{true}
    {}

    VertexArrayObject(const VertexArrayObject&) = delete;
    VertexArrayObject& operator=(const VertexArrayObject&) = delete;

    VertexArrayObject(VertexArrayObject&& other)
    {
        (*this) = std::move(other);
    }

    VertexArrayObject& operator=(VertexArrayObject&& other)
    {
        other.mVertexArrayId = mVertexArrayId;
        other.mActive = false;
        return *this;
    }

    ~VertexArrayObject()
    {
        if (mActive)
        {
            Logging::LogDebug("GLBuffers") << "Deleting GL vertex array id" << mVertexArrayId << "\n";
            glDeleteVertexArrays(1, &mVertexArrayId);
        }
    }

    void BindGL() const
    {
        glBindVertexArray(mVertexArrayId);
    }

    void UnbindGL() const
    {
        glBindVertexArray(0);
    }

    static GLuint GenVertexArrayGL()
    {
        GLuint vertexArrayId;
        glGenVertexArrays(1, &vertexArrayId);
        return vertexArrayId;
    }

private:
    GLuint mVertexArrayId;
    bool mActive;
};

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
            GLBuffer{
                location,
                elems,
                BindPoint::ArrayBuffer,
                GenBufferGL()});

        BindAttribArrayGL(
            location,
            elems,
            mBuffers[name].mBuffer);
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

    ~TextureBuffer()
    {
        glDeleteTextures(1, &mTextureBuffer);
    }

    void BindGL() const
    {
        glBindTexture(mTextureType, mTextureBuffer);
    }

    void UnbindGL() const
    {
        glBindTexture(mTextureType, 0);
    }

    void LoadTexturesGL(
        const std::vector<Texture>& textures,
        unsigned maxDim)
    {
        constexpr auto maxTextures = 256;
        if (textures.size() > maxTextures)
            throw std::runtime_error("Too many textures");

        BindGL();

        glTexStorage3D(
            mTextureType,
            5,              // levels
            GL_RGBA8,       // Internal format
            maxDim, maxDim, // width,height
            maxTextures     // Number of layers
        );
        
        unsigned index = 0;
        for (const auto& tex : textures)
        {
            std::vector<glm::vec4> paddedTex(
                maxDim * maxDim,
                glm::vec4{0, 0, 0, 0});

            // Chuck the image in the padded sized texture
            // GetPixel() will wrap and fill the texture
            for (unsigned x = 0; x < maxDim; x++)
                for (unsigned y = 0; y < maxDim; y++)
                    paddedTex[x + y * maxDim] = tex.GetPixel(x, y);

            glTexSubImage3D(
                mTextureType,
                0,                 // Mipmap number
                0, 0, index,       // xoffset, yoffset, zoffset
                maxDim, maxDim, 1, // width, height, depth
                GL_RGBA,           // format
                GL_FLOAT,          // type
                paddedTex.data()); // pointer to data
            index++;
        }
        
        // Doesn't actually look very good with mipmaps...
        //glGenerateMipmap(mTextureType);
        constexpr auto interpolation = GL_NEAREST;
        //constexpr auto interpolation = GL_LINEAR;
        constexpr auto extend = GL_REPEAT;
        glTexParameteri(mTextureType, GL_TEXTURE_WRAP_S, extend);   
        glTexParameteri(mTextureType, GL_TEXTURE_WRAP_T, extend);
        glTexParameteri(mTextureType, GL_TEXTURE_MIN_FILTER, interpolation);
        glTexParameteri(mTextureType, GL_TEXTURE_MAG_FILTER, interpolation);
        
        UnbindGL();
    }

//private:
    GLuint mTextureBuffer;
    static constexpr GLuint mTextureType = GL_TEXTURE_2D_ARRAY;
};

}
