#pragma once

#include "com/strongType.hpp"

namespace Graphics {

using SpriteSheetIndex = StrongType<unsigned, struct SpriteSheetIndexTag>;
using TextureIndex = StrongType<unsigned, struct TextureIndexTag>;
using MeshOffset = unsigned;
using MeshLength = unsigned;

}
