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

GLenum ToGlEnum(BindPoint p);

class VertexArrayObject
{
public:
    VertexArrayObject();

    VertexArrayObject(const VertexArrayObject&) = delete;
    VertexArrayObject& operator=(const VertexArrayObject&) = delete;

    VertexArrayObject(VertexArrayObject&& other);
    VertexArrayObject& operator=(VertexArrayObject&& other);

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
    GLBuffers(GLBuffers&& other);
    GLBuffers& operator=(GLBuffers&& other);
    GLBuffers(const GLBuffers&) = delete;
    GLBuffers& operator=(const GLBuffers&) = delete;
    ~GLBuffers();

    void AddBuffer(
        const std::string& name,
        unsigned location,
        unsigned elems);
    
    static GLuint GenBufferGL();

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
        GLuint buffer);
    
    void BindArraysGL();
    
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

    TextureBuffer(GLenum textureType);
    TextureBuffer(TextureBuffer&& other);
    TextureBuffer& operator=(TextureBuffer&& other);
    TextureBuffer(const TextureBuffer&) = delete;
    TextureBuffer& operator=(const TextureBuffer&) = delete;

    ~TextureBuffer();
    

    void BindGL() const;
    void UnbindGL() const;
    GLuint GetId() const;
    
    void MakeDepthBuffer(unsigned width, unsigned height);

    void LoadTexturesGL(
        const std::vector<Texture>& textures,
        unsigned maxDim);

//private:
    GLuint mTextureBuffer;
    GLenum mTextureType;
    bool mActive;
};

}
