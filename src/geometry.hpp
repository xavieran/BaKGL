#pragma once

#include "logger.hpp"
#include "tableResource.hpp"

#include "glm.hpp"
#include <glm/glm.hpp>

class Polygon
{
    
    std::vector<glm::vec3> Tesselate()
    {
        unsigned triangles = mIndices.size() - 2;

        std::vector<glm::vec3> vertices;

        for (unsigned triangle = 0; triangle < triangles; triangle++)
        {
            auto i_a = mIndices[0];
            auto i_b = mIndices[triangle + 1];
            auto i_c = mIndices[triangle + 2];

            vertices.emplace_back(mVertices[i_a]);
            vertices.emplace_back(mVertices[i_b]);
            vertices.emplace_back(mVertices[i_c]);
        }
    }

    double HeronsFormula(double a, double b, double c)
    {
        auto s = (a + b + c) * 0.5;
        return glm::sqrt(s * (s - a) * (s - b) * (s - c))
    }

    double GetArea()
    {
        double totalArea = 0.

        auto vertices = Tesselate();
        triangles = vertices.size() / 3;

        const auto GetPoint = [](unsigned t, unsigned p){ return t * 3 + p; };

        for (unsigned triangle = 0; triangle < triangles; triangle++)
        {
            auto a = triangles[GetPoint(0)]
            auto b = triangles[GetPoint(1)]
            auto c = triangles[GetPoint(2)]
            auto area = HeronsFormula(
                glm::distance(a, b), 
                glm::distance(a, c),
                glm::distance(c, b));
            totalArea += area;
        }

        return totalArea;
    }

private:
    const glm::vec3& mVertices;
    std::vector<unsigned> mIndices;
}
