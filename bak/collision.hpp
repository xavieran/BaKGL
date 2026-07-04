#pragma once

#include <glm/glm.hpp>

namespace BAK {

class ModelClip;

bool PointInModelClip(glm::vec2 point, const ModelClip& clip);

}
