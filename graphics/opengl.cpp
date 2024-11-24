#include "graphics/opengl.hpp"

#include "graphics/texture.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include <GL/glew.h>

#include <cmath>

namespace Graphics {

GLenum ToGlEnum(GLBindPoint p)
{
    switch (p)
    {
    case GLBindPoint::ArrayBuffer: return GL_ARRAY_BUFFER;
    case GLBindPoint::ElementArrayBuffer: return GL_ELEMENT_ARRAY_BUFFER;
    case GLBindPoint::TextureBuffer: return GL_TEXTURE_BUFFER;
    default: ASSERT(false); return GL_ARRAY_BUFFER;
    }
}

GLenum ToGlEnum(GLUpdateType p)
{
    switch (p)
    {
    case GLUpdateType::StaticDraw: return GL_STATIC_DRAW;
    case GLUpdateType::DynamicDraw: return GL_DYNAMIC_DRAW;
    default: ASSERT(false); return GL_STATIC_DRAW;
    }
}

VertexArrayObject::VertexArrayObject()
:
    mVertexArrayId{GenVertexArrayGL()},
    mActive{true}
{}

VertexArrayObject::VertexArrayObject(VertexArrayObject&& other) noexcept
{
    (*this) = std::move(other);
}

VertexArrayObject& VertexArrayObject::operator=(VertexArrayObject&& other) noexcept
{
    if (this == &other) return *this;

    Logging::LogDebug(__FUNCTION__) << "Moving to: " << this << " from: " << &other << "\n";
    mVertexArrayId = other.mVertexArrayId;
    other.mActive = false;
    return *this;
}

VertexArrayObject::~VertexArrayObject()
{
    if (mActive)
    {
        Logging::LogDebug("GLBuffers") << "Deleting GL vertex array id: " << mVertexArrayId << "\n";
        glDeleteVertexArrays(1, &mVertexArrayId);
    }
}

void VertexArrayObject::BindGL() const
{
    glBindVertexArray(mVertexArrayId);
}

void VertexArrayObject::UnbindGL() const
{
    glBindVertexArray(0);
}

GLuint VertexArrayObject::GenVertexArrayGL()
{
    GLuint vertexArrayId;
    glGenVertexArrays(1, &vertexArrayId);
    return vertexArrayId;
}

GLBuffers::GLBuffers()
:
    mBuffers{},
    mElementBuffer{GenBufferGL().mValue},
    mActive{true}
{
}

GLBuffers::GLBuffers(GLBuffers&& other) noexcept
{
    (*this) = std::move(other);
}

GLBuffers& GLBuffers::operator=(GLBuffers&& other) noexcept
{
    if (this == &other) return *this;

    for (const auto& [name, buffer] : mBuffers)
        mBuffers.emplace(name, buffer);

    other.mActive = false;

    return *this;
}

GLBuffers::~GLBuffers()
{
    if (mActive)
    {
        Logging::LogDebug("GLBuffers") << "Deleting GL buffers" << std::endl;
        for (const auto& [name, buffer] : mBuffers)
        {
            Logging::LogDebug("GLBuffers") << "    Deleting buffer: " 
                << name << std::endl;
            glDeleteBuffers(1, &buffer.mBuffer.mValue);
        }
    }
}

void GLBuffers::AddBuffer(
    const std::string& name,
    GLLocation location,
    GLElems elems,
    GLDataType dataType,
    GLBindPoint bindPoint,
    GLUpdateType updateType)
{
    mBuffers.emplace(
        name,
        GLBuffer{
            location,
            elems,
            bindPoint,
            dataType,
            updateType,
            GenBufferGL()});

    BindAttribArrayGL(GetGLBuffer(name));
}

void GLBuffers::AddElementBuffer(
    const std::string& name)
{
    AddBuffer(name, GLNullLocation, GLElems{1}, GLDataType{GL_UNSIGNED_INT}, GLBindPoint::ElementArrayBuffer, GLUpdateType::StaticDraw);
}

void GLBuffers::AddTextureBuffer(
    const std::string& name)
{
    AddBuffer(name, GLNullLocation, GLElems{4}, GLDataType{GL_FLOAT}, GLBindPoint::TextureBuffer, GLUpdateType::DynamicDraw);
}


GLBufferId GLBuffers::GenBufferGL()
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    return GLBufferId{buffer};
}

void GLBuffers::BindAttribArrayGL(const GLBuffer& buffer)
{
    if (buffer.mLocation != GLNullLocation)
    {
        glEnableVertexAttribArray(buffer.mLocation.mValue);
        glBindBuffer(ToGlEnum(buffer.mGLBindPoint), buffer.mBuffer.mValue);
        glVertexAttribPointer(
            buffer.mLocation.mValue,
            buffer.mElems.mValue,
            buffer.mDataType.mValue,
            GL_FALSE, // normalized?
            0,        // stride
            (void*) 0 // array buffer offset
        );
    }
    else
    {
        glBindBuffer(ToGlEnum(buffer.mGLBindPoint), buffer.mBuffer.mValue);
    }
}

void GLBuffers::BindArraysGL()
{
    for (const auto& [name, buffer] : mBuffers)
    {
        BindAttribArrayGL(buffer);
    }
}


TextureBuffer::TextureBuffer(
    GLuint textureType)
:
    mTextureBuffer{
        std::invoke([](){
            unsigned texture;
            glGenTextures(1, &texture);
            return texture;
        })
    },
    mTextureType{textureType},
    mActive{true}
{}

TextureBuffer::TextureBuffer(TextureBuffer&& other) noexcept
{
    (*this) = std::move(other);
}

TextureBuffer& TextureBuffer::operator=(TextureBuffer&& other) noexcept
{
    if (this == &other) return *this;
    Logging::LogDebug(__FUNCTION__) << "Moving to: " << this << " from: " << &other << "\n";
    mTextureBuffer = other.mTextureBuffer;
    mTextureType = other.mTextureType;
    other.mActive = false;
    return *this;
}

TextureBuffer::~TextureBuffer()
{
    if (mActive)
    {
        Logging::LogDebug("GLBuffers") << "Deleting GL texture buffer id: " << mTextureBuffer << " @" << this << "\n";
        glDeleteTextures(1, &mTextureBuffer);
    }
}

GLuint TextureBuffer::GetId() const
{
    return mTextureBuffer;
}

void TextureBuffer::BindGL() const
{
    glBindTexture(mTextureType, mTextureBuffer);
}

void TextureBuffer::UnbindGL() const
{
    glBindTexture(mTextureType, 0);
}

void TextureBuffer::MakeDepthBuffer(unsigned width, unsigned height)
{
    ASSERT(mTextureType == GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    BindGL();
    glTexImage2D(mTextureType, 0, GL_DEPTH_COMPONENT,
        width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(mTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(mTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(mTextureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(mTextureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void TextureBuffer::MakePickBuffer(unsigned width, unsigned height)
{
    ASSERT(mTextureType == GL_TEXTURE_2D);
    BindGL();
    glTexImage2D(
        mTextureType, 0,
        GL_RGBA32F,
        width, height, 0,
        GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(mTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(mTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

//void TextureBuffer::MakeTexture2DArray()
//{
//    BindGL();
//
//    glTexStorage3D(
//        mTextureType,
//        1,              // levels
//        GL_RGBA8,       // Internal format
//        maxDim, maxDim, // width,height
//        sMaxTextures     // Number of layers
//    );
//}

void TextureBuffer::LoadTexturesGL(
    const std::vector<Texture>& textures,
    unsigned maxDim)
{
    if (textures.size() > sMaxTextures)
        throw std::runtime_error("Too many textures");

    BindGL();

    glTexStorage3D(
        mTextureType,
        1,              // levels
        GL_RGBA8,       // Internal format
        maxDim, maxDim, // width,height
        sMaxTextures     // Number of layers
    );


    unsigned index = 0;
    for (const auto& tex : textures)
    {
        std::vector<glm::vec4> paddedTex(
            maxDim * maxDim,
            glm::vec4{0});

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
    //constexpr auto extend = GL_CLAMP_TO_BORDER;
    glTexParameteri(mTextureType, GL_TEXTURE_WRAP_S, extend);   
    glTexParameteri(mTextureType, GL_TEXTURE_WRAP_T, extend);
    glTexParameteri(mTextureType, GL_TEXTURE_MIN_FILTER, interpolation);
    glTexParameteri(mTextureType, GL_TEXTURE_MAG_FILTER, interpolation);
    
    UnbindGL();
}

}
