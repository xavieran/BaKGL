#pragma once

#include "graphics/texture.hpp"

#include "com/logger.hpp"
#include "com/strongType.hpp"

#include <GL/glew.h>

namespace Graphics {

using GLLocation = StrongType<unsigned, struct GLLocationTag>;
using GLElems    = StrongType<unsigned, struct GLElemsTag>;
using GLDataType = StrongType<unsigned, struct GLDataTypeTag>;
using GLBufferId = StrongType<unsigned, struct GLBufferIdTag>;

static constexpr auto GLNullLocation = GLLocation{static_cast<unsigned>(-1)};

enum class GLBindPoint
{
    ArrayBuffer,
    ElementArrayBuffer,
    TextureBuffer
};

GLenum ToGlEnum(GLBindPoint);

enum class GLUpdateType
{
    StaticDraw,
    DynamicDraw 
};

GLenum ToGlEnum(GLUpdateType);

struct GLBuffer
{
    // Location in the shader
    GLLocation mLocation;
    // Elements per object (e.g. color = 4 floats)
    GLElems mElems;
    // GLBindPoint (ARRAY, ELEMENT_ARRAY, etc)
    GLBindPoint mGLBindPoint;
    // e.g. GL_FLOAT, or GL_UNSIGNED
    GLDataType mDataType;
    // e.g. StaticDraw or DynamicDraw
    GLUpdateType mUpdateType;
    // GL assigned buffer id
    GLBufferId mBuffer;
};

class VertexArrayObject
{
public:
    VertexArrayObject();

    VertexArrayObject(const VertexArrayObject&) = delete;
    VertexArrayObject& operator=(const VertexArrayObject&) = delete;

    VertexArrayObject(VertexArrayObject&& other) noexcept;
    VertexArrayObject& operator=(VertexArrayObject&& other) noexcept;

    ~VertexArrayObject();

    void BindGL() const;
    void UnbindGL() const;

private:
    static GLuint GenVertexArrayGL();

    GLuint mVertexArrayId;
    bool mActive;
};

class GLBuffers
{
public:
    GLBuffers();

    GLBuffers(GLBuffers&& other) noexcept;
    GLBuffers& operator=(GLBuffers&& other) noexcept;

    GLBuffers(const GLBuffers&) = delete;
    GLBuffers& operator=(const GLBuffers&) = delete;

    ~GLBuffers();

    const auto& GetGLBuffer(const std::string& name) const
    {
        if (!mBuffers.contains(name))
        {
            Logging::LogDebug("GLBuffers") << "No buffer named: " << name << std::endl;
            throw std::runtime_error("Request for nonexistent GL Buffer");
        }
        return mBuffers.find(name)->second;
    }

    void AddBuffer(
        const std::string& name,
        GLLocation,
        GLElems,
        GLDataType,
        GLBindPoint,
        GLUpdateType);

    template <typename T>
    void AddStaticArrayBuffer(
        const std::string& name,
        GLLocation location)
    {
        static_assert(std::is_same_v<typename T::value_type, float> || std::is_same_v<typename T::value_type, unsigned>);
        constexpr auto dataType = std::invoke([](){
            if constexpr (std::is_same_v<typename T::value_type, float>) return GLDataType{GL_FLOAT};
            else return GLDataType{GL_UNSIGNED_INT};
        });
        AddBuffer(name, location, GLElems{T::length()}, dataType, GLBindPoint::ArrayBuffer, GLUpdateType::StaticDraw);
    }

    void AddElementBuffer(const std::string& name);
    void AddTextureBuffer(const std::string& name);
    
    static GLBufferId GenBufferGL();

    template <typename T>
    void LoadBufferDataGL(
        const std::string& name,
        const std::vector<T>& data)
    {
        LoadBufferDataGL(GetGLBuffer(name), data);
    }

    template <typename T>
    void LoadBufferDataGL(
        const GLBuffer& buffer,
        const std::vector<T>& data)
    {
        glBindBuffer(
            ToGlEnum(buffer.mGLBindPoint),
            buffer.mBuffer.mValue);
        glBufferData(
            ToGlEnum(buffer.mGLBindPoint),
            data.size() * sizeof(T),
            &data.front(),
            ToGlEnum(buffer.mUpdateType));
    }

    template <typename T>
    void ModifyBufferDataGL(
        const std::string& name,
        GLenum target,
        unsigned offset,
        const std::vector<T>& data)
    {
        glBindBuffer(target, GetGLBuffer(name).mBuffer.mValue);
        glBufferSubData(
            target,
            offset,
            data.size() * sizeof(T),
            &data.front());
    }

    void BindAttribArrayGL(const GLBuffer&);
    
    void BindArraysGL();

    void SetAttribDivisor(const std::string& name, unsigned divisor)
    {
        const auto location = GetGLBuffer(name).mLocation;
        glEnableVertexAttribArray(location.mValue);
        glVertexAttribDivisor(location.mValue, divisor);
    }

    //private:
    std::unordered_map<std::string, GLBuffer> mBuffers;

    GLuint mElementBuffer;

    // disable when moving from
    bool mActive;
};

class TextureBuffer
{
public:
    static constexpr auto sMaxTextures = 256;

    TextureBuffer(GLenum textureType);
    TextureBuffer(TextureBuffer&& other) noexcept;
    TextureBuffer& operator=(TextureBuffer&& other) noexcept;
    TextureBuffer(const TextureBuffer&) = delete;
    TextureBuffer& operator=(const TextureBuffer&) = delete;

    ~TextureBuffer();
    

    void BindGL() const;
    void UnbindGL() const;
    GLuint GetId() const;
    
    void MakeDepthBuffer(unsigned width, unsigned height);
    void MakeTexture2DArray();

    void LoadTexturesGL(
        const std::vector<Texture>& textures,
        unsigned maxDim);

//private:
    GLuint mTextureBuffer;
    GLenum mTextureType;
    bool mActive;
};

}
