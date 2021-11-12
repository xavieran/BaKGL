#pragma once

#include "graphics/opengl.hpp"

#include <GL/glew.h>

namespace Graphics {

class FrameBuffer
{
public:
    FrameBuffer();

    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    FrameBuffer(FrameBuffer&& other);
    FrameBuffer& operator=(FrameBuffer&& other);

    ~FrameBuffer();

    void BindGL() const;
    void UnbindGL() const;
    void AttachDepthTexture(const Graphics::TextureBuffer&) const;

private:
    static GLuint GenFrameBufferGL();

    GLuint mFrameBufferId;
    bool mActive;
};

}
