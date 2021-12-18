#pragma once

#include <glm/glm.hpp>

#include <functional>
#include <string>
#include <vector>

namespace BAK {

class Palette;

class ColorSwap
{
public:
    static constexpr auto sSize = 256;

    ColorSwap(const std::string& filename);

    const glm::vec4& GetColor(unsigned i, const Palette&) const;

private:
    std::vector<unsigned> mIndices;
};
class Palette
{
public:
    Palette(const std::string& filename);

    Palette(const Palette& pal, const ColorSwap& cs)
    :
        mColors{std::invoke([&](){
            auto swappedPal = std::vector<glm::vec4>{};
            for (unsigned i = 0; i < 256; i++)
            {
                swappedPal.emplace_back(cs.GetColor(i, pal));
            }
            return swappedPal;
        })}
    {
    }

    const glm::vec4& GetColor(unsigned i) const;

private:
    std::vector<glm::vec4> mColors;
};


}
