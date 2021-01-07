#pragma once

#include <GL/glew.h>

#include <cmath>

namespace BAK {

class GLBuffers
{
public:
    GLBuffers()
    :
        mVertexBuffer{GenGLBuffer()},
        mNormalBuffer{GenGLBuffer()},
        mColorBuffer{GenGLBuffer()},
        mTextureCoordBuffer{GenGLBuffer()},
        mTextureBlendBuffer{GenGLBuffer()},
        mElementBuffer{GenGLBuffer()}
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
    
    static GLuint GenGLBuffer()
    {
        GLuint buffer;
        glGenBuffers(1, &buffer);
        return buffer;
    }
    
    template <typename T>
    void LoadBufferData(GLuint buffer, GLenum target, const std::vector<T>& data)
    {
        glBindBuffer(target, buffer);
        glBufferData(
            target,
            data.size() * sizeof(T),
            &data.front(),
            // This will need to change...
            GL_STATIC_DRAW);
    }
    
    GLuint mVertexBuffer;
    GLuint mNormalBuffer;
    GLuint mColorBuffer;
    GLuint mTextureCoordBuffer;
    GLuint mTextureBlendBuffer;
    GLuint mElementBuffer;
};

}
