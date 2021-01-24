#pragma once


// BAK coordinate system
// 
// Origin is at 0
// positive x goes right
// positive y goes up
//
// e.g.
//
//                            T1117 (736'000, 1'120'000)
// T1016 (672'000, 1'056'000) T1116 (736'000, 1'056'000)
//
// Need to convert this to open gl system by swapping y and z// and negating z (depth goes towards us)

#include <glm/glm.hpp>

namespace BAK {

glm::vec3 ToGlCoord(const glm::vec3& coord)
{
    return glm::vec3{coord.x, -coord.z, coord.y};
}

template <typename T, typename C>
glm::vec<4, T> ToGlColor(const C& color, bool transparent)
{
    const auto F = [](auto x){
        return static_cast<T>(x) / 256.; };

    return glm::vec<4, T>{
        F(color.r),
        F(color.g),
        F(color.b),
        transparent ? 0 : 1};
}

}
