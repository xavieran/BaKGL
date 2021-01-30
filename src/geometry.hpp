#pragma once

#include "logger.hpp"
#include "tableResource.hpp"
#include "worldFactory.hpp"

#include "glm.hpp"
#include <glm/glm.hpp>

class Polygon
{
public:
    Polygon(const BAK::ZoneItem& zoneItem, unsigned face)
    :
        mVertices{std::invoke([&](){
            auto vertices = std::vector<glm::vec3>{};
            for (auto i : zoneItem.GetFaces().at(face))
                vertices.emplace_back(zoneItem.GetVertices().at(i));
            return vertices;
        })},
        mFace{face}
    {
    }

    const auto& GetVertices() const { return mVertices; }
    const auto& GetFace() const { return mFace; }

    std::vector<glm::vec3> Tesselate() const
    {
        unsigned triangles = mVertices.size() - 2;

        std::vector<glm::vec3> vertices;

        for (unsigned triangle = 0; triangle < triangles; triangle++)
        {
            vertices.emplace_back(mVertices[0]);
            vertices.emplace_back(mVertices[triangle + 1]);
            vertices.emplace_back(mVertices[triangle + 2]);
        }

        return vertices;
    }

    bool Coplanar(const Polygon& other) const
    {
        return GetNormal() == other.GetNormal();
    }

    static bool VectorCmp(glm::vec3 v1, glm::vec3 v2)
    {
        auto diff = v1 - v2;
        for (unsigned i = 0; i < 3; i++)
        {
            if (diff[i] == 0.) continue;
            return diff[i] < 0.;
        }
        return false;
    }

    glm::vec3 GetNormal() const
    {
        auto a = mVertices[0];
        auto b = mVertices[1];
        auto c = mVertices[2];

        auto v1 = a - b;
        auto v2 = a - c;

        return glm::normalize(glm::cross(v1, v2));
    }

    static double HeronsFormula(double a, double b, double c)
    {
        auto s = (a + b + c) * 0.5;
        return glm::sqrt(s * (s - a) * (s - b) * (s - c));
    }

    double GetArea() const
    {
        double totalArea = 0;

        auto vertices = Tesselate();
        unsigned triangles = vertices.size() / 3;

        const auto GetPoint = [](unsigned t, unsigned p){ return t * 3 + p; };

        for (unsigned triangle = 0; triangle < triangles; triangle++)
        {
            auto a = vertices[GetPoint(triangle, 0)];
            auto b = vertices[GetPoint(triangle, 1)];
            auto c = vertices[GetPoint(triangle, 2)];

            auto area = HeronsFormula(
                glm::distance(a, b), 
                glm::distance(a, c),
                glm::distance(c, b));

            totalArea += area;
        }

        return totalArea;
    }

private:
    std::vector<glm::vec3> mVertices;
    unsigned mFace;
};


