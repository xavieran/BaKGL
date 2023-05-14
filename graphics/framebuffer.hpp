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

    FrameBuffer(FrameBuffer&& other) noexcept;
    FrameBuffer& operator=(FrameBuffer&& other) noexcept;

    ~FrameBuffer();

    void BindGL() const;
    void UnbindGL() const;
    void AttachDepthTexture(const Graphics::TextureBuffer&, bool clearDrawBuffer) const;
    void AttachTexture(const Graphics::TextureBuffer&) const;

private:
    static GLuint GenFrameBufferGL();

    GLuint mFrameBufferId;
    bool mActive;
};

}
