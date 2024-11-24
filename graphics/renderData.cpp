#include "graphics/renderData.hpp"

#include "graphics/meshObject.hpp"
#include "graphics/opengl.hpp"
#include "graphics/framebuffer.hpp"
#include "graphics/shaderProgram.hpp"

#include "com/logger.hpp"

namespace Graphics {

RenderData::RenderData()
:
    mVertexArrayObject{},
    mGLBuffers{},
    mTextureBuffer{GL_TEXTURE_2D_ARRAY}
{
}

void RenderData::LoadData(
    const MeshObjectStorage& objectStore,
    const std::vector<Texture>& textures,
    unsigned maxDimension)
{
    Logging::LogInfo(__FUNCTION__) << "Loading render data. Textures: " << textures.size() << " max dim: " << maxDimension << "\n";
    // FIXME Issue 48: Need to do destruct and restruct all the buffers
    // when we load new data...
    mVertexArrayObject.BindGL();

    mGLBuffers.AddStaticArrayBuffer<glm::vec3>("vertex", GLLocation{0});
    mGLBuffers.AddStaticArrayBuffer<glm::vec3>("normal", GLLocation{1});
    mGLBuffers.AddStaticArrayBuffer<glm::vec4>("color", GLLocation{2});
    mGLBuffers.AddStaticArrayBuffer<glm::vec3>("textureCoord", GLLocation{3});
    mGLBuffers.AddStaticArrayBuffer<glm::vec1>("textureBlend", GLLocation{4});
    mGLBuffers.AddElementBuffer("elements");

    mGLBuffers.LoadBufferDataGL("vertex", objectStore.mVertices);
    mGLBuffers.LoadBufferDataGL("normal", objectStore.mNormals);
    mGLBuffers.LoadBufferDataGL("color", objectStore.mColors);
    mGLBuffers.LoadBufferDataGL("textureCoord", objectStore.mTextureCoords);
    mGLBuffers.LoadBufferDataGL("textureBlend", objectStore.mTextureBlends);
    mGLBuffers.LoadBufferDataGL("elements", objectStore.mIndices);

    mGLBuffers.BindArraysGL();

    mTextureBuffer.LoadTexturesGL(
        textures,
        maxDimension);
}

void RenderData::Bind(GLuint textureTarget) const
{
    mVertexArrayObject.BindGL();
    glActiveTexture(textureTarget);
    mTextureBuffer.BindGL();
}

}
