#pragma once

#include <glm/glm.hpp>
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

namespace Graphics {

template <typename T>
bool PointWithinRectangle(glm::vec<2, T> point, glm::vec<2, T> topLeft, glm::vec<2, T> dimensions)
{
    const auto bottomRight = topLeft + dimensions;
    return glm::all(glm::greaterThanEqual(point, topLeft))
        && glm::all(glm::lessThanEqual(point, bottomRight));
}

}
