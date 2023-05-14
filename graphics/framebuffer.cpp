#include "graphics/framebuffer.hpp"

#include "com/logger.hpp"


namespace Graphics {

FrameBuffer::FrameBuffer()
:
    mFrameBufferId{GenFrameBufferGL()},
    mActive{true}
{
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
{
    (*this) = std::move(other);
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept
{
    other.mFrameBufferId = mFrameBufferId;
    other.mActive = false;
    return *this;
}

FrameBuffer::~FrameBuffer()
{
    if (mActive)
    {
        Logging::LogDebug("GLBuffers") << "Deleting GL frame buffer id: " << mFrameBufferId << "\n";
        glDeleteFramebuffers(1, &mFrameBufferId);
    }
}

void FrameBuffer::BindGL() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
}

void FrameBuffer::UnbindGL() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::AttachDepthTexture(const Graphics::TextureBuffer& buffer, bool clearDrawBuffer) const
{
    BindGL();
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        buffer.GetId(),
        0);
    if (clearDrawBuffer)
    {
        glDrawBuffer(GL_NONE);
    }
    glReadBuffer(GL_NONE);
    UnbindGL();
}

void FrameBuffer::AttachTexture(const Graphics::TextureBuffer& buffer) const
{
    BindGL();
    glFramebufferTexture(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        buffer.GetId(),
        0);
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);
    UnbindGL();
}

GLuint FrameBuffer::GenFrameBufferGL()
{
    GLuint framebufferId;
    glGenFramebuffers(1, &framebufferId);
    return framebufferId;
}

}
