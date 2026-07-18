#pragma once

#include <glm/glm.hpp>

#include <optional>

namespace BAK {

struct ClipElement;
struct ModelClip;
class ZoneItem;

bool PointInClipElement(glm::vec2 point, const ClipElement& element);
bool PointInModelClip(glm::vec2 point, const ModelClip&);
bool BlocksMovement(const ZoneItem&);
bool AllowsMovement(const ZoneItem&);

std::optional<float> ComputeHeight(
    glm::vec2 modelClipPos,
    const ClipElement& element);

std::optional<float> ComputeHeight(
    glm::vec2 modelClipPos,
    const ModelClip& clip);

float ComputeWorldHeight(
    float height,
    float itemScale);

glm::vec2 WorldToModelClipSpace(
    glm::vec2 playerPos,
    glm::vec2 itemPos,
    float rotation,
    float scale);

glm::vec2 ModelClipToWorldSpace(
    glm::vec2 playerPos,
    glm::vec2 itemPos,
    float rotation,
    float scale);

}
