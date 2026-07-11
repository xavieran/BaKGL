#pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

namespace glm {

template <int Size, typename T>
std::ostream& operator<<(std::ostream& os, const glm::vec<Size, T>& x)
{
    os << glm::to_string(x);
    return os;
}

template <typename S, typename T>
vec<2, S> cast(const vec<2, T>& x)
{
    return static_cast<vec<2, S>>(x);
}

template <typename S, typename T>
vec<3, S> cast(const vec<3, T>& x)
{
    return static_cast<vec<3, S>>(x);
}

}

struct UVec2Hash {
    std::size_t operator()(const glm::uvec2& v) const {
        auto h1 = std::hash<unsigned>{}(v.x);
        auto h2 = std::hash<unsigned>{}(v.y);
        return h1 ^ (h2 << 1);
    }
};

namespace Graphics {

template <typename T>
bool PointWithinRectangle(glm::vec<2, T> point, glm::vec<2, T> topLeft, glm::vec<2, T> dimensions)
{
    const auto bottomRight = topLeft + dimensions;
    return glm::all(glm::greaterThanEqual(point, topLeft))
        && glm::all(glm::lessThanEqual(point, bottomRight));
}

constexpr auto sNinetyDegreeRotation = glm::vec3{0, glm::pi<float>() * .5, 0};

inline glm::mat4 CalculateModelMatrix(
    const glm::vec3& location,
    const glm::vec3& scale,
    const glm::vec3& rotation,
    float worldScale)
{
    auto modelMatrix = glm::mat4{1.0};
    modelMatrix = glm::translate(modelMatrix, location / worldScale);
    modelMatrix = glm::scale(modelMatrix, scale);
    // Dodgy... only works for rotation about y
    modelMatrix = glm::rotate(
        modelMatrix,
        rotation.y,
        glm::vec3{0, 1, 0});
    return modelMatrix;
}

}
