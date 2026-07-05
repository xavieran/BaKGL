#pragma once

#include <glm/glm.hpp>

namespace BAK {

struct ModelClip;
class ZoneItem;

bool PointInModelClip(glm::vec2 point, const ModelClip&);
bool BlocksMovement(const ZoneItem&);
bool AllowsMovement(const ZoneItem&);
glm::vec2 WorldToModelClipSpace(glm::vec2 playerPos, glm::vec2 itemPos, float rotation, float scale);

}
