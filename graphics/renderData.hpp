#pragma once

#include "graphics/opengl.hpp"

namespace Graphics {
class MeshObjectStorage;
class Texture;

class RenderData
{
public:
    RenderData();

    void LoadData(
        const MeshObjectStorage& objectStore,
        const std::vector<Texture>& textures,
        unsigned maxDimension);

    void Bind(GLuint textureTarget) const;
private:
    VertexArrayObject mVertexArrayObject;
    GLBuffers mGLBuffers;
    TextureBuffer mTextureBuffer;
};

}
